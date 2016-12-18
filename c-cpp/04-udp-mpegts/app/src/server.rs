use std;
use std::fs::File;
use std::io::{Write, Read};
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
                if ready.is_readable() {
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
                                match e {
                                    http::RequestError::NoData => {}, // ok
                                    _ => println!("error reading HTTP request: {}", e),
                                }
                            },
                            Ok(..) => {
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
                        }
                    }
                }

                if ready.is_writable() {
                    {
                        let client = &self.clients[&token];
                        let mut conn = &client.conn;
                        println!("[<-] [w] {{\
                            \"fd\": {}\
                            , \"token\": {}\
                            , \"ready\": \"{:?}\"\
                        }}", conn.as_raw_fd(), usize::from(token), ready);

                        let mut http_resp = http::Response::new();
                        let mut http_resp_body = String::new();

                        match client.http_request.method() {
                            http::Method::GET => {
                                match client.http_request.path().as_ref() {
                                    "/" => {
                                        let mut f = File::open("./static/index.html").unwrap();
                                        f.read_to_string(&mut http_resp_body).unwrap();
                                        http_resp.header_mut().set("Content-Type".to_string(), "text/html; charset=UTF-8".to_string());
                                    },
                                    "/ws/v1" => {
                                        println!("websocket!");
                                    },

                                    "/static/app.js" => {
                                        let mut f = File::open("./static/app.js").unwrap();
                                        f.read_to_string(&mut http_resp_body).unwrap();
                                        http_resp.header_mut().set("Content-Type".to_string(), "text/javascript; charset=UTF-8".to_string());
                                    },
                                    "/static/app.css" => {
                                        let mut f = File::open("./static/app.css").unwrap();
                                        f.read_to_string(&mut http_resp_body).unwrap();
                                        http_resp.header_mut().set("Content-Type".to_string(), "text/css; charset=UTF-8".to_string());
                                    },

                                    _ => {
                                        let mut f = File::open("./static/404.html").unwrap();
                                        f.read_to_string(&mut http_resp_body).unwrap();
                                        http_resp.set_status(http::Status::NotFound);
                                        http_resp.header_mut().set("Content-Type".to_string(), "text/html; charset=UTF-8".to_string());
                                    }
                                }
                            }
                            _ => {
                                let mut f = File::open("./static/405.html").unwrap();
                                f.read_to_string(&mut http_resp_body).unwrap();
                                http_resp.set_status(http::Status::MethodNotAllowed);
                                http_resp.header_mut().set("Content-Type".to_string(), "text/html; charset=UTF-8".to_string());
                            }
                        }

                        http_resp.set_content_length(http_resp_body.as_bytes().len());

                        match conn
                            .write(http_resp.to_string().as_bytes())
                            .and_then(|_| conn.write(http_resp_body.as_bytes())) {
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
                    }

                    let client = self.clients.get_mut(&token).unwrap();
                    client.http_request_reset();
                }

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
                }
            }
        }

        Err(ServerError::Stop)
    }
}
