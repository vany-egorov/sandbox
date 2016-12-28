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
    copy
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

impl State {
    #[inline]
    pub fn is_http_request(&self) -> bool {
        match *self {
            State::HTTP(Some(..), None) => true,
            _ => false,
        }
    }

    #[inline]
    pub fn is_http_response(&self) -> bool {
        match *self {
            State::HTTP(Some(..), Some(..)) => true,
            _ => false,
        }
    }
}


pub struct Connection {
    pub token: Token,
    pub sock: TcpStream,
    pub handler: Handler,

    pub state: State,

    pub buf: Cursor<Vec<u8>>, // incoming data
}


impl Connection {
    pub fn new(token: Token, sock: TcpStream, handler: Handler) -> Connection {
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
    pub fn token_u64(&self) -> u64 { self.token_usz() as u64 }
    pub fn sock(&self) -> &TcpStream { &self.sock }

    pub fn do_read(&mut self) -> Result<()> {
        // <~~~>
        // TODO: refactor cursor + vec-buffer
        // TODO: refactor state switching
        self.buf.set_position(0);
        self.buf.get_mut().clear();
        self.state = State::Accepted;
        // </~~~>

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
        if self.handler.is_http() && self.state.is_http_request() {
            self.on_http_response();
        }

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
        self.handler.on_tcp_read();
    }

    pub fn on_http_request(&mut self, req: HTTPRequest) {
        self.state = State::HTTP(Some(req), None);
        self.handler.on_http_request();
    }

    pub fn on_http_response(&mut self) -> Result<()> {
        let id = self.token_u64();
        let mut resp = HTTPResponse::new();
        let mut resp_body = Cursor::new(Vec::with_capacity(1024));

        if let State::HTTP(Some(ref req), _) = self.state {
            self.handler.on_http_response(id, req, &mut resp, &mut resp_body);
        }

        resp.set_content_length(resp_body.position() as usize);

        self.sock
            .write(resp.to_string().as_bytes())
            .and_then(|_| self.sock.write(&resp_body.into_inner()));

        if let State::HTTP(Some(ref req), ref mut _resp) = self.state {
            self.handler.on_http_response_after(id, req, &resp);
            *_resp = Some(resp);
        }

        Ok(())
    }
}
