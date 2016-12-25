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


pub struct Server {
    poll: Poll,
    connections: Slab<Connection, Token>,

    listener: Option<TcpListener>,

    events: mio::Events,

    tx: mio::channel::SyncSender<()>,
    rx: mio::channel::Receiver<()>,

    state: ServerState,
}

impl Server {
    pub fn new(settings: ServerSettings) -> Result<Server> {
        let (tx, rx) = mio::channel::sync_channel(settings.sync_channel_bound);

        Ok(Server {
            poll: try!(Poll::new()),
            connections: Slab::with_capacity(settings.max_connections),

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
        let c = try!(TcpListener::bind(&addr));

        try!(self.poll.register(
              &c
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

        self.listener = Some(c);

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
        println!("[+]");
    }
}
