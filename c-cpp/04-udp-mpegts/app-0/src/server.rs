extern crate sha1;
extern crate ws;
extern crate rustc_serialize;

use std;
use std::fs::File;
use std::io::{Write, Read, Cursor};
use std::collections::HashMap;
use std::os::unix::io::AsRawFd;
use std::time::Duration;

use mio;
use mio::{Poll, Token, Ready, PollOpt, Events};
use mio::tcp::{TcpListener, Shutdown};
use self::rustc_serialize::base64::ToBase64;

use http;
use client;


const SERVER: Token = Token(std::usize::MAX - 2);
const QUEUE:  Token = Token(std::usize::MAX - 3);
const TIMER:  Token = Token(std::usize::MAX - 4);


#[derive(Debug, Clone)]
pub enum Signal {
    A,
    B
}

#[derive(Debug, Clone)]
pub struct Command {
    pub data: Vec<u8>,
    pub signal: Signal,
}

impl Command {
    pub fn into_signal(self) -> Signal {
        self.signal
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Timeout {
    connection: Token,
    event: Token,
}


#[derive(Debug)]
pub enum ServerError {
    Stop
}

pub struct Server {
    pub token: Token,
    pub clients: HashMap<Token, client::Client>,

    pub tx: mio::channel::SyncSender<Command>,
    pub rx: mio::channel::Receiver<Command>,
    pub timer: mio::timer::Timer<Timeout>,
}

// TODO: move somewhere
static WS_GUID: &'static str = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
fn gen_key(key: &String) -> String {
    let mut m = sha1::Sha1::new();

    m.update(key.as_bytes());
    m.update(WS_GUID.as_bytes());

    return m.digest().bytes().to_base64(rustc_serialize::base64::STANDARD);
}

impl Server {
    pub fn new() -> Server {
        let (tx, rx) = mio::channel::sync_channel(100 * 5);
        let timer = mio::timer::Builder::default()
            .tick_duration(Duration::from_millis(1000))
            .num_slots(1024)
            .capacity(65_536)
            .build();

        Server {
            token: SERVER,
            clients: HashMap::new(),
            tx: tx,
            rx: rx,
            timer: timer,
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

        println!("queue-token: {:?}", QUEUE);
        println!("timer-token: {:?}", TIMER);

        mio_poll.register(
              &server_sock
            , self.token
            , Ready::readable()
            , PollOpt::level()
        ).unwrap();
        mio_poll.register(&self.rx, QUEUE, Ready::readable(), PollOpt::edge() | PollOpt::oneshot()).unwrap();
        mio_poll.register(&self.timer, TIMER, Ready::readable(), PollOpt::edge()).unwrap();

        let mut events = Events::with_capacity(1024);
        let mut token_id_seq: usize = 1;

        loop {
            mio_poll.poll(&mut events, None).unwrap();

            for event in events.iter() {
                let token = event.token();
                let ready = event.kind();

                match token {
                    SERVER => { // event on server listening socket
                        if ready.is_readable() {
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
                        }
                    },
                    QUEUE => {
                        for _ in 0..256 { // 256 == MESSAGES_PER_TICK
                            match self.rx.try_recv() {
                                Ok(cmd) => {
                                    match cmd.signal {
                                        Signal::A => {
                                            // println!("[->] [ws] {:?}", cmd.data);
                                            let mut frame = ws::Frame::message(cmd.data.into(), ws::OpCode::Binary, true);
                                            let mut encoded = Vec::new();
                                            frame.format(&mut encoded);

                                            for (token, client) in self.clients.iter_mut() {
                                                if client.state == client::State::WS {
                                                    // println!("[->] [ws] {:?}", token);
                                                    client.conn.write(encoded.as_slice());
                                                }
                                            };
                                        },
                                        Signal::B => println!("queue {:?}", cmd),
                                    }
                                }
                                _ => break
                            }
                        }
                        mio_poll.reregister(&self.rx, QUEUE, Ready::readable(), PollOpt::edge() | PollOpt::oneshot()).unwrap();
                    },
                    TIMER => { println!("timer"); },
                    _ => {
                        if ready.is_readable() {
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

                            match client.state  {
                                client::State::Accepted | client::State::HTTPRs => {
                                    match client.http_request_read() {
                                        Err(e) => {
                                            match e {
                                                http::RequestError::NoData => {}, // ok
                                                _ => {
                                                    println!("error reading HTTP request: {}", e);

                                                    client.state = client::State::TCP;
                                                    match mio_poll.reregister(
                                                          &client.conn
                                                        , token
                                                        , Ready::readable() | Ready::hup()
                                                        ,   PollOpt::edge()
                                                          | PollOpt::oneshot()
                                                    ) {
                                                        Err(e) => println!("error register client socket: {}", e),
                                                        Ok(..) => {}
                                                    }
                                                },
                                            }
                                        },
                                        Ok(..) => {
                                            client.state = client::State::HTTPRq;

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
                                client::State::WS => {
                                    let mut data = Vec::new();
                                    client.conn.read_to_end(&mut data);

                                    let mut buf = Cursor::new(data);

                                    while let Some(mut frame) = ws::Frame::parse(&mut buf).unwrap() {
                                        frame.remove_mask();
                                        if frame.is_final() {
                                            match frame.opcode() {
                                                ws::OpCode::Text => {
                                                    let s = String::from_utf8(frame.into_data()).unwrap();
                                                    println!("[<-] [ws] {}", s);
                                                },
                                                _ => { println!("!!!!!!!!!!!!!!"); }
                                            }
                                        }
                                    }

                                    match mio_poll.reregister(
                                          &client.conn
                                        , token
                                        , Ready::readable() | Ready::hup()
                                        ,   PollOpt::edge()
                                          | PollOpt::oneshot()
                                    ) {
                                        Err(e) => println!("error register client socket: {}", e),
                                        Ok(..) => {}
                                    }
                                },
                                _ => {
                                    println!("read {:?}", client.state);
                                }
                            }
                        }

                        if ready.is_writable() {
                            {
                                let client = self.clients.get_mut(&token).unwrap();
                                let mut conn = &client.conn;
                                println!("[<-] [w] {{\
                                    \"fd\": {}\
                                    , \"token\": {}\
                                    , \"ready\": \"{:?}\"\
                                }}", conn.as_raw_fd(), usize::from(token), ready);

                                let mut http_resp = http::Response::new();
                                let mut http_resp_body = String::new();

                                if client.state == client::State::HTTPRq {
                                    client.state = client::State::HTTPRs;
                                }

                                match client.http_request.method() {
                                    http::Method::GET => {
                                        match client.http_request.path().as_ref() {
                                            "/" => {
                                                let mut f = File::open("./static/index.html").unwrap();
                                                f.read_to_string(&mut http_resp_body).unwrap();
                                                http_resp.header_mut().set("Content-Type".to_string(), "text/html; charset=UTF-8".to_string());
                                            },
                                            "/ws/v1" => {
                                                client.state = client::State::WS;
                                                let response_key = gen_key(&client.http_request.header.get_first("Sec-WebSocket-Key".to_string()).unwrap());

                                                http_resp.set_status(http::Status::SwitchingProtocols);
                                                http_resp.header_mut().set("Upgrade".to_string(), "websocket".to_string());
                                                http_resp.header_mut().set("Connection".to_string(), "Upgrade".to_string());
                                                http_resp.header_mut().set("Sec-WebSocket-Accept".to_string(), response_key);
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
            }
        }

        Err(ServerError::Stop)
    }
}
