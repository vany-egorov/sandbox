use std;
use std::io::Write;
use std::collections::HashMap;
use std::os::unix::io::AsRawFd;

use mio::{Poll, Token, Ready, PollOpt, Events};
use mio::tcp::{TcpListener, Shutdown};

use http;
use client;


#[derive(Debug)]
pub enum ServerError {
    Stop
}


pub struct Server {
    pub token: Token,
    pub clients: HashMap<Token, client::Client>,
}

impl Server {
    pub fn new() -> Server {
        Server {
            token: Token(0),
            clients: HashMap::new(),
        }
    }

    pub fn listen_and_serve(&mut self, addr_raw: &str) -> Result<(), ServerError> {
        let addr: std::net::SocketAddr = match addr_raw.parse() {
            Ok(v) => v,
            Err(e) => {
                println!("error parsing addr \"{}\": \"{}\"", addr_raw, e);
                std::process::exit(1);
            }
        };

        let server_sock = match TcpListener::bind(&addr) {
            Ok(v) => v,
            Err(e) => {
                println!("error starting tcp server using addr \"{}\": \"{}\"", addr_raw, e);
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
            , self.token
            , Ready::readable()
            , PollOpt::edge()
        ).unwrap();

        let mut events = Events::with_capacity(1024);
        let mut token_id_seq: usize = 1;

        loop {
            mio_poll.poll(&mut events, None).unwrap();

            for event in events.iter() {
                let token = event.token();
                let ready = event.kind();
                if ready.is_hup() || ready.is_error() {
                    {
                        let client = &self.clients[&token];
                        let conn = &client.conn;
                        println!("[<-] [h] {{\
                            \"fd\": {}\
                            , \"token\": {}\
                            , \"ready\": \"{:?}\"\
                        }}", conn.as_raw_fd(), usize::from(token), ready);

                        match mio_poll.deregister(conn) {
                            Err(e) => println!("[->] [h] deregister error: {}", e),
                            Ok(..) => println!("[->] [h] deregister OK")
                        };
                        match conn.shutdown(Shutdown::Both) {
                            Err(e) => println!("[->] [h] shutdown error: {}", e),
                            Ok(..) => println!("[->] [h] shutdown OK")
                        }
                    }
                    self.clients.remove(&token);
                } else if ready.is_readable() {
                    if token == self.token { // event on server listening socket
                        // creates a new connected socket
                        // and returns a new file descriptor
                        // referring to that socket
                        let conn = match server_sock.accept() {
                            Err(e) => {
                                println!("error accepting connection: {}", e);
                                continue;
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

                        let token_client = Token(token_id_seq);
                        token_id_seq += 1;

                        let client = client::Client::new(token_client, conn);
                        self.clients.insert(client.token, client);

                        println!("[<-] [+] {{\
                            \"fd\": {}\
                            , \"token\": {}\
                            , \"ready\": \"{:?}\"\
                        }}", &self.clients[&token_client].conn.as_raw_fd(), usize::from(token_client), ready);

                        match mio_poll.register(
                              &self.clients[&token_client].conn
                            , token_client
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
                    } else {
                        let client = self.clients.get_mut(&token).unwrap();
                        println!("[<-] [r] {{\
                                \"fd\": {}\
                                , \"token\": {}\
                                , \"ready\": \"{:?}\"\
                            }}",
                            client.conn.as_raw_fd(),
                            usize::from(token),
                            ready
                        );

                        match client.http_request_read() {
                            Err(e) => {
                                println!("error reading HTTP request: {}", e);
                                continue;
                            },
                            Ok(..) => {}
                        }

                        match mio_poll.reregister(
                              &client.conn
                            , token
                            , Ready::writable() | Ready::hup()
                            ,   PollOpt::edge()
                              | PollOpt::oneshot()
                        ) {
                            Err(e) => println!("error register client socket: {}", e),
                            Ok(..) => {}
                        }
                    }
                } else if ready.is_writable() {
                    let client = &self.clients[&token];
                    let mut conn = &client.conn;
                    println!("[<-] [w] {{\
                        \"fd\": {}\
                        , \"token\": {}\
                        , \"ready\": \"{:?}\"\
                    }}", conn.as_raw_fd(), usize::from(token), ready);

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
                    match conn.write(resp_raw.as_bytes()) {
                        Err(e) => println!("[->] [w] write error: {}", e),
                        Ok(len) => {
                            println!("[->] [w] {{\
                                \"fd\": {}\
                                , \"token\": {}\
                                , \"written\": {}\
                            }}", conn.as_raw_fd(), usize::from(token), len);

                            println!("[<-] {} {}", client.http_request.method, client.http_request.url_raw);
                            println!("{}", client.http_request);

                            match mio_poll.reregister(
                                  conn
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

        Err(ServerError::Stop)
    }
}
