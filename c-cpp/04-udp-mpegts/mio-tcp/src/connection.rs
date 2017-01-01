use std::io::{
    Read,
    Write,
    Result as IoResult,
    ErrorKind as IoErrorKind,
    Cursor,
};

use mio::Token;
use mio::tcp::TcpStream;

use result::Result;
use error::Error;
use router::Router;
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

    #[inline]
    pub fn is_ws(&self) -> bool {
        match *self {
            State::WS => true,
            _ => false,
        }
    }
}


pub struct Connection {
    pub token: Token,
    pub sock: TcpStream,
    pub handler: Option<Handler>,

    pub state: State,

    pub buf: Cursor<Vec<u8>>, // incoming data
}


impl Connection {
    pub fn new(token: Token, sock: TcpStream) -> Connection {
        Connection {
            token: token,
            sock: sock,

            state: State::Pending,
            handler: None,

            buf: Cursor::new(Vec::with_capacity(2048)),
        }
    }

    pub fn token(&self) -> Token { self.token }
    pub fn token_usz(&self) -> usize { usize::from(self.token) }
    pub fn token_u64(&self) -> u64 { self.token_usz() as u64 }
    pub fn sock(&self) -> &TcpStream { &self.sock }

    #[inline]
    fn is_handler_http(&self) -> bool {
        if let Some(ref handler) = self.handler {
            if handler.is_http() {
                return true;
            }
        }

        false
    }

    #[inline]
    fn is_handler_ws(&self) -> bool {
        if let Some(ref handler) = self.handler {
            if handler.is_ws() {
                return true;
            }
        }

        false
    }

    pub fn do_read<R>(&mut self, router: &mut R) -> Result<()>
        where R: Router
    {
        let id = self.token_u64();
        let sz = try!(map_non_block(self.sock.read_to_end(&mut self.buf.get_mut())))
            .unwrap_or_else(|| self.buf.get_ref().len());

        match self.state {
            State::Accepted => {
                match HTTPRequest::decode_from(&mut self.buf) {
                    Err(err) => {
                        match err {
                            HTTPRequestError::RequestLineMissing | HTTPRequestError::Utf8(..) => { // plain TCP data
                                self.on_tcp_read(sz);
                            },
                            _ => { return Err(Error::from(err)); },
                        }
                    },
                    Ok(opt) => {
                        match opt {
                            Some(req) => { // got HTTP request
                                self.on_tcp_read(sz);
                                self.buf.set_position(req.header_length as u64); // move cursor forward
                                self.on_http_request(router, req);
                            },
                            None => {},
                        }
                    }
                }
            },
            State::WS => {
                if self.is_handler_ws() {
                    if let Some(ref mut handler) = self.handler {
                        handler.on_ws_message(id, sz);
                    }
                }
                self.reset_buffer()
            },
            _ => {
                // TODO: handler error - bad state
            }
        }

        Ok(())
    }

    pub fn do_write(&mut self) -> Result<()> {
        if self.state.is_http_request() {
            self.on_http_response();

            if self.is_handler_http() {
                self.reset();
            } else if self.is_handler_ws() {
                self.state = State::WS;
                // clear HTTP request header + HTTP request body
                self.reset_buffer();
            }
        }

        Ok(())
    }

    pub fn on_tcp_accept(&mut self) {
        self.state = State::Accepted;

        debug!("[+] {{\"token\": {token}, \"event\": \"accept\"}}",
            token=self.token_usz());
    }

    // reset connection state and buffers
    // between HTTP Request/Response calls
    fn reset(&mut self) {
        self.reset_buffer();

        // TCP accept state
        self.state = State::Accepted;

        // drop previously routed handler
        self.handler = None;
    }

    fn reset_buffer(&mut self) {
        // clear TCP/HTTP buffer
        self.buf.set_position(0);
        self.buf.get_mut().clear();
    }

    fn on_tcp_read(&mut self, sz: usize) {
        self.state = State::TCP;

        debug!("[<] {{\"token\": {token}, \"event\": \"{size} bytes readen\"}}",
            token=self.token_usz(), size=sz);
    }

    fn on_http_request<R>(&mut self, router: &mut R, req: HTTPRequest)
        where R: Router
    {
        self.state = State::HTTP(Some(req), None);

        if let State::HTTP(Some(ref req), _) = self.state {
            let mut handler = router.route(&req);
            handler.on_http_request(&req);
            self.handler = Some(handler);
        }
    }

    fn on_http_response(&mut self) -> Result<()> {
        let id = self.token_u64();
        let mut resp = HTTPResponse::new();
        let mut resp_body = Cursor::new(Vec::with_capacity(1024));

        if let State::HTTP(Some(ref req), _) = self.state {
            if let Some(ref mut handler) = self.handler {
                handler.on_http_response(id, req, &mut resp, &mut resp_body);
            }
        }

        let content_length = resp_body.position() as usize;
        resp.set_content_length(content_length);

        try!(
            self.sock
            .write(resp.to_string().as_bytes())
            .and_then(|_| self.sock.write(&resp_body.into_inner()))
        );

        if let State::HTTP(Some(ref req), ref mut _resp) = self.state {
            if let Some(ref mut handler) = self.handler {
                handler.on_http_response_after(id, req, &resp);
                *_resp = Some(resp);
            }
        }

        Ok(())
    }
}
