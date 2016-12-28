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
use mio::tcp::{
    TcpListener,
    Shutdown
};
use slab::Slab;

use tokens;
use result::Result;
use error::{Error, Kind as ErrorKind};
use factory::Factory;
use connection::Connection;
use server_settings::ServerSettings;


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
    connections: Slab<Connection, Token>,

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
                if let Err(err) = self.on_event(evt.token(), evt.kind()) {
                    error!("Failed handle event: {:?}", err);
                }
            }
        }

        Ok(())
    }

    fn on_event(&mut self, token: Token, events: Ready) -> Result<()> {
        match token {
            tokens::SERVER => {
                if events.is_readable() {
                    let token = try!(self.do_accept());
                    let conn = try!(
                        self.connections
                            .get_mut(token)
                            .ok_or(Error::new(ErrorKind::NoConnectionAssociatedWithToken, ""))
                    );

                    conn.on_tcp_accept();
                }
            },
            tokens::BUS => {},
            _ => {
                {
                    let conn = try!(
                        self.connections
                            .get_mut(token)
                            .ok_or(Error::new(ErrorKind::NoConnectionAssociatedWithToken, ""))
                    );

                    if events.is_readable() {
                        try!(conn.do_read());

                        try!(self.poll.reregister(
                              conn.sock()
                            , conn.token()
                            , Ready::writable() | Ready::error() | Ready::hup()
                            , PollOpt::edge() | PollOpt::oneshot()
                        ))
                    }

                    if events.is_writable() {
                       try!(conn.do_write());

                       try!(self.poll.reregister(
                              conn.sock()
                            , conn.token()
                            , Ready::readable() | Ready::error() | Ready::hup()
                            , PollOpt::edge() | PollOpt::oneshot()
                        ))
                    }
                }

                if events.is_hup() {
                    try!(self.do_hup(token));
                    self.factory.on_hup();
                }

                if events.is_error() {
                    try!(self.do_error(token));
                    self.factory.on_error();
                }
            },
        }

        Ok(())
    }

    fn do_accept(&mut self) -> Result<Token> {
        let token = match self.listener.as_ref() {
            Some(server_sock) => {
                let (sock, _) = try!(server_sock.accept());

                let token = {
                    if let Some(entry) = self.connections.vacant_entry() {
                        let token = entry.index();
                        let handler = self.factory.on_accept(token);
                        entry.insert(Connection::new(token, sock, handler));
                        token
                    } else {
                        return Err(Error::new(ErrorKind::Capacity, "Unable to add another TCP connection to the event loop while accepting TCP connection"));
                    }
                };

                if let Err(err) = {
                    let conn = &self.connections[token];
                    self.poll.register(
                          conn.sock()
                        , conn.token()
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

    fn do_hup(&mut self, token: Token) -> Result<()> {
        if let Some(conn) = self.connections.get_mut(token) {
            try!(self.poll.deregister(conn.sock()));
            try!(conn.sock().shutdown(Shutdown::Both));
        }

        // TODO: trigger handler on-tcp-hup

        self.connections.remove(token);

        Ok(())
    }

    fn do_error(&mut self, token: Token) -> Result<()> {
        if let Some(conn) = self.connections.get_mut(token) {
            try!(self.poll.deregister(conn.sock()));
            try!(conn.sock().shutdown(Shutdown::Both));
        }

        // TODO: trigger handler on-tcp-error

        self.connections.remove(token);

        Ok(())
    }
}
