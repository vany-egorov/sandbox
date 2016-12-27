use std;
use std::fs::File;
use std::io::{
    Read,
    Write,
    Result as IoResult,
    ErrorKind as IoErrorKind,
    Cursor,
    Seek,
    SeekFrom,
};

use mio::{
    Poll,
    Token,
    Ready,
    PollOpt,
};
use mio::tcp::TcpStream;

use result::Result;
use error::Error;
use handler::Handler;
use http::{
    Request as HTTPRequest,
    RequestError as HTTPRequestError,
    Response as HTTPResponse,
};


pub fn map_non_block<T>(res: IoResult<T>) -> IoResult<Option<T>> {
    match res {
        Ok(value) => Ok(Some(value)),
        Err(err) => {
            if let IoErrorKind::WouldBlock = err.kind() {
                Ok(None)
            } else {
                Err(err)
            }
        }
    }
}


pub enum State {
    Pending,
    // TCP? HTTP? WS?
    Accepted,

    // plain TCP
    TCP,
    HTTP(Option<HTTPRequest>, Option<HTTPResponse>),
    WS,

    Disconnected,
}


pub struct Connection<H>
    where H: Handler
{
    pub token: Token,
    pub sock: TcpStream,
    pub handler: H,

    pub state: State,

    pub buf: Cursor<Vec<u8>>, // incoming data
}


impl<H> Connection<H>
    where H: Handler
{
    pub fn new(token: Token, sock: TcpStream, handler: H) -> Connection<H> {
        Connection {
            token: token,
            sock: sock,
            handler: handler,

            state: State::Pending,

            buf: Cursor::new(Vec::with_capacity(2048)),
        }
    }

    pub fn token(&self) -> Token { self.token }
    pub fn token_usz(&self) -> usize { usize::from(self.token) }
    pub fn sock(&self) -> &TcpStream { &self.sock }

    pub fn do_read(&mut self) -> Result<()> {
        let sz = try!(map_non_block(self.sock.read_to_end(&mut self.buf.get_mut())))
            .unwrap_or_else(|| self.buf.get_ref().len());

        debug!("[<] {{\"token\": {token}, \"event\": \"{size} bytes readen\"}}",
            token=self.token_usz(), size=sz);

        match HTTPRequest::decode_from(&mut self.buf) {
            Err(err) => {
                match err {
                    HTTPRequestError::RequestLineMissing | HTTPRequestError::Utf8(..) => { // plain TCP data
                        self.on_tcp_read();
                    },
                    _ => { return Err(Error::from(err)); },
                }
            },
            Ok(opt) => {
                match opt {
                    Some(req) => { // got HTTP request
                        self.on_tcp_read();
                        self.buf.set_position(req.header_length as u64); // move cursor forward
                        self.on_http_request(req);
                    },
                    None => {},
                }
            }
        }

        Ok(())
    }

    pub fn do_write(&mut self) -> Result<()> {
        let mut http_resp = HTTPResponse::new();
        let mut http_resp_body = String::new();

        let mut f = File::open("./static/index.html").unwrap();
        f.read_to_string(&mut http_resp_body).unwrap();

        http_resp.header_mut().set("Content-Type".to_string(), "text/html; charset=UTF-8".to_string());
        http_resp.set_content_length(http_resp_body.as_bytes().len());

        self.sock.write(http_resp.to_string().as_bytes())
            .and_then(|_| self.sock.write(http_resp_body.as_bytes()));

        Ok(())
    }

    pub fn on_tcp_accept(&mut self) {
        // conn: trigger tcp-accept
        self.state = State::Accepted;

        debug!("[+] {{\"token\": {token}, \"event\": \"accept\"}}",
            token=self.token_usz());
    }

    pub fn on_tcp_read(&mut self) {
        // conn: trigger tcp-read
        self.state = State::TCP;
    }

    pub fn on_http_request(&mut self, req: HTTPRequest) {
        // conn: trigger http-request
        self.state = State::HTTP(Some(req), None);
        if let State::HTTP(Some(ref req), _) = self.state {
            println!("{}", req);
        }
    }

    pub fn on_http_response(&mut self, req: HTTPRequest) {
        // conn: trigger http-request
        self.state = State::HTTP(Some(req), None);

        debug!("[<] {{\"token\": {token}, \"event\": \"HTTP request\"}}",
            token=self.token_usz());

        if let State::HTTP(Some(ref req), _) = self.state {
            print!("{}", req);
        }
    }
}
