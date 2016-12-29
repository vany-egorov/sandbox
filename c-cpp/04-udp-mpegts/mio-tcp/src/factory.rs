use mio::Token;

use http::Request;
use handler::Handler;
use connection::Connection;



pub trait Factory {
    fn on_tcp_accept(&mut self, token: Token) {
        debug!("factory -> on-tcp-accept;")
    }

    fn on_http_route(&self, req: &Request) -> Handler;

    fn on_tcp_hup(&mut self) {
        debug!("factory -> on-tcp-hup;")
    }
    fn on_tcp_error(&mut self) {
        debug!("factory -> on-tcp-error;")
    }
}

impl<F> Factory for F
    where F: Fn(&Request) -> Handler
{
    fn on_http_route(&self, req: &Request) -> Handler { self(req) }
}
