extern crate libc;
extern crate mio;
extern crate http_muncher;
#[macro_use]
extern crate chan;
extern crate chan_signal;
extern crate sha1;
extern crate rustc_serialize;

use std::fmt;
use std::io::Read;
use mio::TryRead;
use mio::TryWrite;
use libc::{c_int, c_void};
use chan_signal::Signal;
use std::collections::HashMap;
use rustc_serialize::base64::{ToBase64, STANDARD};
use std::cell::RefCell;
use std::rc::Rc;

mod va;


#[derive(PartialEq)]
enum ClientState {
    AwaitingHandshake,
    HandshakeResponse,
    Connected
}

struct HttpParser {
    current_key: Option<String>,
    headers: Rc<RefCell<HashMap<String, String>>>
}

impl http_muncher::ParserHandler for HttpParser {
    fn on_header_field(&mut self, s: &[u8]) -> bool {
        self.current_key = Some(std::str::from_utf8(s).unwrap().to_string());
        true
    }

    fn on_header_value(&mut self, s: &[u8]) -> bool {
        self.headers.borrow_mut()
            .insert(self.current_key.clone().unwrap(),
                    std::str::from_utf8(s).unwrap().to_string());
        true
    }

    fn on_headers_complete(&mut self) -> bool {
        false
    }
}

fn gen_key(key: &String) -> String {
    let mut m = sha1::Sha1::new();
    let mut buf = [0u8; 20];

    m.update(key.as_bytes());
    m.update("258EAFA5-E914-47DA-95CA-C5AB0DC85B11".as_bytes());

    m.output(&mut buf);

    return buf.to_base64(rustc_serialize::base64::STANDARD);
}

struct WebSocketClient {
    socket: mio::tcp::TcpStream,
    http_parser: http_muncher::Parser<HttpParser>,
    headers: Rc<RefCell<HashMap<String, String>>>,
    interest: mio::EventSet,
    state: ClientState
}

impl WebSocketClient {
    fn new(socket: mio::tcp::TcpStream) -> WebSocketClient {
        let headers = Rc::new(RefCell::new(HashMap::new()));

        WebSocketClient {
            socket: socket,
            headers: headers.clone(),
            interest: mio::EventSet::readable(),
            state: ClientState::AwaitingHandshake,

            http_parser: http_muncher::Parser::request(HttpParser {
                current_key: None,
                headers: headers.clone()
            })
        }
    }

    fn read(&mut self) {
        loop {
            let mut buf = [0; 2048];
            match self.socket.try_read(&mut buf) {
                Err(e) => {
                    println!("read error: {}", e);
                    return
                },
                Ok(None) => {
                    println!("Socket buffer has got no more bytes;");
                    break;
                },
                Ok(Some(len)) => {
                    let s = std::str::from_utf8(&buf).unwrap();
                    println!("{} | {:?}", len, s);
                    self.http_parser.parse(&buf[0..len]);
                    if self.http_parser.is_upgrade() {
                        self.state = ClientState::HandshakeResponse;

                        self.interest.remove(mio::EventSet::readable());
                        self.interest.insert(mio::EventSet::writable());

                        break;
                    }
                }
            }
        }
    }

    fn write(&mut self) {
        // Заимствуем хеш-таблицу заголовков из контейнера Rc<RefCell<...>>:
        let headers = self.headers.borrow();

        // Находим интересующий нас заголовок и генерируем ответный ключ используя его значение:
        let response_key = gen_key(&headers.get("Sec-WebSocket-Key").unwrap());

        // Мы используем специальную функцию для форматирования строки.
        // Ее аналоги можно найти во многих других языках (printf в Си, format в Python, и т.д.),
        // но в Rust есть интересное отличие - за счет макросов форматирование происходит во время
        // компиляции, и в момент выполнения выполняется только уже оптимизированная "сборка" строки
        // из кусочков. Мы обсудим использование макросов в одной из следующих частей этой статьи.
        let response = fmt::format(format_args!("HTTP/1.1 101 Switching Protocols\r\n\
                                                 Connection: Upgrade\r\n\
                                                 Sec-WebSocket-Accept: {}\r\n\
                                                 Upgrade: websocket\r\n\r\n", response_key));

        // Запишем ответ в сокет:
        self.socket.try_write(response.as_bytes()).unwrap();

        // Снова изменим состояние клиента:
        self.state = ClientState::Connected;

        // И снова поменяем набор интересующих нас событий на `readable()` (на чтение):
        self.interest.remove(mio::EventSet::writable());
        self.interest.insert(mio::EventSet::readable());
    }
}

struct WebSocketServer {
    socket: mio::tcp::TcpListener,
    clients: HashMap<mio::Token, WebSocketClient>,
    token_counter: usize
}

const SERVER_TOKEN: mio::Token = mio::Token(0);

impl mio::Handler for WebSocketServer {
    type Timeout = usize;
    type Message = ();

    fn ready(&mut self, event_loop: &mut mio::EventLoop<WebSocketServer>,
             token: mio::Token, events: mio::EventSet)
    {
        if events.is_readable() {
            match token {
                SERVER_TOKEN => {
                    println!("1 => {:?}", SERVER_TOKEN);
                    let client_socket = match self.socket.accept() {
                        Err(e) => {
                            println!("connection error: {}", e);
                            return;
                        },
                        Ok(None) => unreachable!(),
                        Ok(Some((sock, addr))) => sock
                    };

                    self.token_counter += 1;
                    let new_token = mio::Token(self.token_counter);

                    self.clients.insert(new_token, WebSocketClient::new(client_socket));
                    event_loop.register(&self.clients[&new_token].socket,
                                            new_token, mio::EventSet::readable(),
                                            mio::PollOpt::edge() | mio::PollOpt::oneshot()).unwrap();
                }
                token => {
                    println!("2 => {:?}", token);
                    let mut client = self.clients.get_mut(&token).unwrap();
                    client.read();
                    event_loop.reregister(&client.socket, token, client.interest,
                                          mio::PollOpt::edge() | mio::PollOpt::oneshot()).unwrap();
                }
            }
        }

        if events.is_writable() {
            let mut client = self.clients.get_mut(&token).unwrap();
            client.write();
            event_loop.reregister(&client.socket, token, client.interest,
                                  mio::PollOpt::edge() | mio::PollOpt::oneshot()).unwrap();
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
