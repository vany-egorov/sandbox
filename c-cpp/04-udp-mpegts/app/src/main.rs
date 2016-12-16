extern crate mio;
extern crate libc;
#[macro_use]
extern crate chan;
#[macro_use]
extern crate lazy_static;
extern crate chan_signal;

use std::io::Write;
use std::os::unix::io::AsRawFd;
use std::collections::HashMap;

use mio::{Poll, Token, Ready, PollOpt, Events};
use mio::tcp::{TcpListener, TcpStream};

use libc::{c_int, c_void};
use chan_signal::Signal;

mod va;
mod http;


const ADDR_RAW: &'static str = "0.0.0.0:8000";
const SERVER_TOKEN: Token = Token(0);


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

    let addr: std::net::SocketAddr = match ADDR_RAW.parse() {
        Ok(v) => v,
        Err(e) => {
            println!("error parsing addr \"{}\": \"{}\"", ADDR_RAW, e);
            std::process::exit(1);
        }
    };

    let server_sock = match TcpListener::bind(&addr) {
        Ok(v) => v,
        Err(e) => {
            println!("error starting tcp server using addr \"{}\": \"{}\"", ADDR_RAW, e);
            std::process::exit(1);
        }
    };

    let mio_poll = match Poll::new() {
        Ok(v) => v,
        Err(e) => {
            println!("error create poll: \"{}\"", e);
            std::process::exit(1);
        }
    };

    mio_poll.register(
          &server_sock
        , SERVER_TOKEN
        , Ready::readable()
        , PollOpt::edge()
    ).unwrap();

    let mut events = Events::with_capacity(1024);
    std::thread::spawn(move || {
        let mut token_id: usize = 1;
        let mut clients: HashMap<Token, TcpStream> = HashMap::with_capacity(5);

        loop {
            mio_poll.poll(&mut events, None).unwrap();

            for event in events.iter() {
                let token = event.token();
                let ready = event.kind();
                if ready.is_hup() {
                    {
                        let client_tcp_stream = &clients[&token];
                        println!("[<-] [h] {{\
                            \"fd\": {}\
                            , \"token\": {}\
                            , \"ready\": \"{:?}\"\
                        }}", client_tcp_stream.as_raw_fd(), usize::from(token), ready);

                        match mio_poll.deregister(client_tcp_stream) {
                            Err(e) => println!("[->] [h] deregister error: {}", e),
                            Ok(..) => println!("[->] [h] deregister OK")
                        };
                        match client_tcp_stream.shutdown(mio::tcp::Shutdown::Both) {
                            Err(e) => println!("[->] [h] shutdown error: {}", e),
                            Ok(..) => println!("[->] [h] shutdown OK")
                        }
                    }
                    clients.remove(&token);
                } else if ready.is_readable() {
                    match token {
                        // event on server listening socket
                        SERVER_TOKEN => {
                            // creates a new connected socket
                            // and returns a new file descriptor
                            // referring to that socket
                            let client_tcp_stream = match server_sock.accept() {
                                Err(e) => {
                                    println!("error accepting connection: {}", e);
                                    return;
                                },
                                Ok((tcp_stream, addr)) => {
                                    println!("[<-] [+] {{\
                                        \"fd\": {}\
                                        , \"server-token\": {}\
                                        , \"addr\": \"{}\"\
                                        , \"ready\": \"{:?}\"\
                                    }}", tcp_stream.as_raw_fd(), usize::from(token), addr, ready);
                                    tcp_stream
                                },
                            };

                            let client_token = Token(token_id);
                            token_id += 1;
                            clients.insert(client_token, client_tcp_stream);
                            println!("[<-] [+] {{\
                                \"fd\": {}\
                                , \"token\": {}\
                                , \"ready\": \"{:?}\"\
                            }}", &clients[&client_token].as_raw_fd(), usize::from(client_token), ready);
                            match mio_poll.register(
                                  &clients[&client_token]
                                , client_token
                                , Ready::readable() | Ready::hup()
                                , PollOpt::edge() // trigger event only once;
                                                  // "level" will trigger (spam) event until
                                                  // got data to read in socket;

                                  | PollOpt::oneshot() // one time; otherwise got events on close socket;
                                                       // i.e. on ctrl+C while curl;
                            ) {
                                Err(e) => println!("error register client socket: {}", e),
                                Ok(..) => {}
                            }
                        }
                        token => {
                            let mut client_tcp_stream = &clients[&token];
                            println!("[<-] [r] {{\
                                    \"fd\": {}\
                                    , \"token\": {}\
                                    , \"ready\": \"{:?}\"\
                                }}",
                                client_tcp_stream.as_raw_fd(),
                                usize::from(token),
                                ready
                            );

                            let mut req = http::Request::new();
                            req.decode_from(&mut client_tcp_stream);
                            print!("{}", req);

                            match mio_poll.reregister(
                                  client_tcp_stream
                                , token
                                , Ready::writable() | Ready::hup()
                                ,   PollOpt::edge()
                                  | PollOpt::oneshot()
                            ) {
                                Err(e) => println!("error register client socket: {}", e),
                                Ok(..) => {}
                            }
                        }
                    }
                } else if ready.is_writable() {
                    let mut client_tcp_stream = &clients[&token];
                    println!("[<-] [w] {{\
                        \"fd\": {}\
                        , \"token\": {}\
                        , \"ready\": \"{:?}\"\
                    }}", client_tcp_stream.as_raw_fd(), usize::from(token), ready);

                    let resp_body = "{\"foo\":\"bar\"}";
                    let resp_raw = std::fmt::format(
                        format_args!(
                            "HTTP/1.1 {http_status_code} {http_status_text}\r\n\
                            Content-Type: application/json\r\n\
                            Content-Length: {content_length}\r\n\
                            \r\n\
                            {body}",
                            http_status_code=http::Status::OK as u16,
                            http_status_text=http::Status::OK,
                            content_length=resp_body.as_bytes().len(),
                            body=resp_body
                        )
                    );
                    match client_tcp_stream.write(resp_raw.as_bytes()) {
                        Err(e) => println!("[->] [w] write error: {}", e),
                        Ok(len) => {
                            println!("[->] [w] {{\
                                \"fd\": {}\
                                , \"token\": {}\
                                , \"written\": {}\
                            }}", client_tcp_stream.as_raw_fd(), usize::from(token), len);

                            match mio_poll.reregister(
                                  client_tcp_stream
                                , token
                                ,   Ready::readable() | Ready::none() | Ready::hup()
                                ,   PollOpt::edge()
                                  | PollOpt::oneshot()
                            ) {
                                Err(e) => println!("error register client socket: {}", e),
                                Ok(..) => {}
                            }
                        }
                    }
                } else {
                    panic!("{:?} {:?}", ready, token);
                }
            }
        }
    });

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
