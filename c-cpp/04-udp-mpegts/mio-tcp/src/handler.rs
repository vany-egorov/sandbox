use std::io::Write;

use handler_tcp::HandlerTCP;
use handler_http::HandlerHTTP;
use handler_ws::HandlerWS;
use http::{
    Request as HTTPRequest,
    Response as HTTPResponse,
};


pub enum Handler {
    TCP(Box<HandlerTCP>),
    HTTP(Box<HandlerHTTP>),
    WS(Box<HandlerWS>),
}

impl Handler {
    #[inline]
    pub fn is_tcp(&self) -> bool {
        match *self {
            Handler::TCP(..) => true,
            _ => false,
        }
    }

    #[inline]
    pub fn is_http(&self) -> bool {
        match *self {
            Handler::HTTP(..) => true,
            _ => false,
        }
    }

    #[inline]
    pub fn is_ws(&self) -> bool {
        match *self {
            Handler::WS(..) => true,
            _ => false,
        }
    }

    pub fn on_tcp_read(&mut self) {
        match *self {
            Handler::TCP(ref mut h)  => h.on_tcp_read(),
            Handler::HTTP(ref mut h) => h.on_tcp_read(),
            Handler::WS(ref mut h)   => h.on_tcp_read(),
        }
    }

    pub fn on_http_request(&mut self) {
        match *self {
            Handler::TCP(ref mut h)  => { },
            Handler::HTTP(ref mut h) => h.on_http_request(),
            Handler::WS(ref mut h)   => h.on_http_request(),
        }
    }

    pub fn on_http_response(&mut self, id: u64, req: &HTTPRequest, resp: &mut HTTPResponse, w: &mut Write) {
        match *self {
            Handler::TCP(ref mut h)  => { },
            Handler::HTTP(ref mut h) => h.on_http_response(id, req, resp, w),
            Handler::WS(ref mut h)   => h.on_http_response(id, req, resp, w),
        }
    }

    pub fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        match *self {
            Handler::TCP(ref mut h)  => { },
            Handler::HTTP(ref mut h) => h.on_http_response_after(id, req, resp),
            Handler::WS(ref mut h)   => h.on_http_response_after(id, req, resp),
        }
    }
}
