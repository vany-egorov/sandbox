use std::usize;
use std::net::SocketAddr;

use mio;
use mio::{
    Poll,
    Token,
    Ready,
    PollOpt,
    Events,
};
use mio::tcp::TcpListener;
use slab::Slab;

use tokens;
use result::Result;
use error::{Error, Kind as ErrorKind};
use factory::Factory;
use connection::Connection;
use server_settings::ServerSettings;


type Conn<F> = Connection<<F as Factory>::Handler>;


#[derive(Debug, PartialEq, Eq)]
enum ServerState {
    On,
    Off,
}

impl ServerState {
    #[inline]
    fn is_on(&self) -> bool {
        match *self {
            ServerState::On  => true,
            ServerState::Off => false,
        }
    }
}


pub struct Server<F>
    where F: Factory
{
    poll: Poll,
    connections: Slab<Conn<F>, Token>,

    factory: F,

    listener: Option<TcpListener>,

    events: mio::Events,

    tx: mio::channel::SyncSender<()>,
    rx: mio::channel::Receiver<()>,

    state: ServerState,
}

impl<F> Server<F>
    where F: Factory
{
    pub fn new(settings: ServerSettings, factory: F) -> Result<Server<F>> {
        let (tx, rx) = mio::channel::sync_channel(settings.sync_channel_bound);

        Ok(Server {
            poll: try!(Poll::new()),
            connections: Slab::with_capacity(settings.max_connections),

            factory: factory,

            listener: None,

            events: Events::with_capacity(settings.events_cap),

            tx: tx,
            rx: rx,

            state: ServerState::Off,
        })
    }

    pub fn listen_and_serve(&mut self, addr: &SocketAddr) -> Result<()> {
        try!(self.listen(addr));
        try!(self.serve());

        Ok(())
    }

    pub fn listen(&mut self, addr: &SocketAddr) -> Result<()> {
        let sock = try!(TcpListener::bind(&addr));

        try!(self.poll.register(
              &sock
            , tokens::SERVER
            , Ready::readable()
            , PollOpt::level()
        ));

        try!(self.poll.register(
              &self.rx
            , tokens::BUS
            , Ready::readable()
            , PollOpt::edge() | PollOpt::oneshot()
        ));

        self.listener = Some(sock);

        info!("TCP server starting on {}", &addr);

        Ok(())
    }

    pub fn serve(&mut self) -> Result<()> {
        self.state = ServerState::On;
        self.event_loop().unwrap(); // TODO: handle
        self.state = ServerState::Off;

        Ok(())
    }

    fn event_loop(&mut self) -> Result<()> {
        while self.state.is_on() {
            let nevents = self.poll.poll(&mut self.events, None).unwrap(); // TODO: handle

            for i in 0..nevents {
                let evt = self.events.get(i).unwrap();
                self.on_event(evt.token(), evt.kind());
            }
        }

        Ok(())
    }

    fn on_event(&mut self, token: Token, events: Ready) {
        match token {
            tokens::SERVER => {
                if events.is_readable() {
                    match self.accept() {
                        Err(err)  => error!("Unable to accept client connection: {:?}", err),
                        Ok(token) => {
                            debug!("[+] {{\"event\": \"accept\", \"token\": {token}}}",
                                token=usize::from(token));

                        },
                    }
                }
            },
            tokens::BUS => {},
            _ => {
                if events.is_readable() {
                    println!("[r]");
                }

                if events.is_writable() {
                   println!("[w]");
                }
            },
        }
    }

    fn accept(&mut self) -> Result<Token> {
        let token = match self.listener.as_ref() {
            Some(server_sock) => {
                let (sock, _) = try!(server_sock.accept());

                let token = {
                    if let Some(entry) = self.connections.vacant_entry() {
                        let token = entry.index();
                        let handler = self.factory.produce(token);
                        entry.insert(Connection::new(token, sock, handler));
                        token
                    } else {
                        return Err(Error::new(ErrorKind::Capacity, "Unable to add another TCP connection to the event loop"));
                    }
                };

                if let Err(err) = {
                    let conn = &self.connections[token];
                    self.poll.register(
                          &conn.sock
                        , conn.token
                        , Ready::readable() | Ready::hup()
                        , PollOpt::edge() | PollOpt::oneshot()
                    )
                } {
                    self.connections.remove(token);
                    return Err(Error::from(err));
                }

                token
            },
            None => return Err(Error::new(ErrorKind::NoListenerProvided, "while accepting TCP connection")),
        };

        Ok(token)
    }
}
