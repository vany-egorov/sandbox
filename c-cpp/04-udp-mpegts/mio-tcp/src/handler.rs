use std::io::Write;

use handler_http::HandlerHTTP;
use handler_ws::HandlerWS;
use http::{
    Request as HTTPRequest,
    Response as HTTPResponse,
};


pub enum Handler {
    HTTP(Box<HandlerHTTP>),
    WS(Box<HandlerWS>),
}

impl Handler {
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

    // pub fn route(&mut self, req: &HTTPRequest) -> Option<Handler> {
    //     if let Handler::HTTPRouter(ref mut h) = *self {
    //         return Some(h.route(req));
    //     }

    //     None
    // }

    pub fn on_http_request(&mut self, req: &HTTPRequest) {
        match *self {
            Handler::HTTP(ref mut h) => h.on_http_request(req),
            Handler::WS(ref mut h) => h.on_http_request(req),
        }
    }

    pub fn on_http_response(&mut self, id: u64, req: &HTTPRequest, resp: &mut HTTPResponse, w: &mut Write) {
        match *self {
            Handler::HTTP(ref mut h) => h.on_http_response(id, req, resp, w),
            Handler::WS(ref mut h) => h.on_http_response(id, req, resp, w),
        }
    }

    pub fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        match *self {
            Handler::HTTP(ref mut h) => h.on_http_response_after(id, req, resp),
            Handler::WS(ref mut h) => h.on_http_response_after(id, req, resp),
        }
    }
}
