extern crate libc;
extern crate mio;
extern crate http_muncher;
#[macro_use]
extern crate chan;
extern crate chan_signal;

use libc::{c_int, c_void};
use chan_signal::Signal;
use std::collections::HashMap;

mod va;


struct HttpParser;
impl http_muncher::ParserHandler for HttpParser { }

struct WebSocketServer {
    socket: mio::tcp::TcpListener,
    clients: HashMap<mio::Token, mio::tcp::TcpStream>,
    token_counter: usize
}

const SERVER_TOKEN: mio::Token = mio::Token(0);

impl mio::Handler for WebSocketServer {
    type Timeout = usize;
    type Message = ();

    fn ready(&mut self, event_loop: &mut mio::EventLoop<WebSocketServer>,
             token: mio::Token, events: mio::EventSet)
    {
        match token {
            SERVER_TOKEN => {
                let client_socket = match self.socket.accept() {
                    Err(e) => {
                        println!("connection error: {}", e);
                        return;
                    },
                    Ok(None) => panic!("Accept has returned 'None'"),
                    Ok(Some((sock, addr))) => sock
                };

                self.token_counter += 1;
                let new_token = mio::Token(self.token_counter);

                self.clients.insert(new_token, client_socket);
                event_loop.register(&self.clients[&new_token],
                                        new_token, mio::EventSet::readable(),
                                        mio::PollOpt::edge() | mio::PollOpt::oneshot()).unwrap();
            }
            token => { println!("{:?}", token); }
        }
    }
}


unsafe extern "C" fn va_parser_parse_cb(ctx: *mut c_void, atom: *mut c_void, atom_kind: u32, offset: u64) -> c_int {
    if atom_kind == 0 { return 0; }
    println!("0x{:08X} | {:p} | {:p} | {}", offset, ctx, atom, atom_kind);
    return 0;
}

fn main() {
    let signal = chan_signal::notify(&[Signal::INT, Signal::TERM]);
    let raw = std::ffi::CString::new("udp://239.1.1.1:5500").unwrap();

    let mut va_parser: va::Parser = Default::default();
    let va_parser_open_args = va::ParserOpenArgs{
        i_url_raw : raw.as_ptr(),
        cb : Some(va_parser_parse_cb),
    };

    let address = "0.0.0.0:8000".parse::<std::net::SocketAddr>().unwrap();
    let server_socket = mio::tcp::TcpListener::bind(&address).unwrap();

    let mut event_loop = mio::EventLoop::new().unwrap();
    let mut server = WebSocketServer{
        token_counter: 1,
        clients: HashMap::new(),
        socket: server_socket
    };

    event_loop.register(&server.socket,
                    SERVER_TOKEN,
                    mio::EventSet::readable(),
                    mio::PollOpt::edge()).unwrap();

    event_loop.run(&mut server).unwrap();

    unsafe {
        va::va_parser_open(&mut va_parser, &va_parser_open_args);
        va::va_parser_go(&mut va_parser);
    }

    chan_select! {
        signal.recv() -> signal => {
            println!("received signal: {:?}", signal)
        }
    }
}
