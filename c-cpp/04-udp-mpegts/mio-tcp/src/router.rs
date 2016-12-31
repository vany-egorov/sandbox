use mio::Token;

use http::Request;
use handler::Handler;


pub trait Router {
    fn on_tcp_accept(&mut self, _: Token) { debug!("factory -> on-tcp-accept;") }

    // before handler.on_http_request
    fn route(&mut self, req: &Request) -> Handler;

    fn on_tcp_hup(&mut self) {
        debug!("factory -> on-tcp-hup;")
    }
    fn on_tcp_error(&mut self) {
        debug!("factory -> on-tcp-error;")
    }
}

impl<F> Router for F
    where F: FnMut(&Request) -> Handler
{
    fn route(&mut self, req: &Request) -> Handler {
        let handler = self(req);
        handler
    }
}
