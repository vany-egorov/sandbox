use mio::Token;

use handler::Handler;
use connection::Connection;



pub trait Factory {
    fn on_accept(&mut self, token: Token) -> Handler;
    fn on_hup(&mut self) {
        debug!("factory -> on-hup;")
    }
}

impl<F> Factory for F
    where F: FnMut(Token) -> Handler
{
    fn on_accept(&mut self, token: Token) -> Handler { self(token) }
}
