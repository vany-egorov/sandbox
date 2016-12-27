use handler_tcp::HandlerTCP;
use http::{
    Handler as HandlerHTTP,
    Request as HTTPRequest,
    Response as HTTPResponse,
};
use ws::Handler as HandlerWS;


pub enum Handler {
    TCP(Box<HandlerTCP>),
    HTTP(Box<HandlerHTTP>),
    WS(Box<HandlerWS>),
}

impl Handler {
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

    pub fn on_http_response(&mut self, req: &HTTPRequest, resp: &mut HTTPResponse) {
        match *self {
            Handler::TCP(ref mut h)  => { },
            Handler::HTTP(ref mut h) => h.on_http_response(req, resp),
            Handler::WS(ref mut h)   => h.on_http_response(req, resp),
        }
    }
}
