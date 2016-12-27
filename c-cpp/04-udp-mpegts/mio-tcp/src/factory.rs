use mio::Token;

use handler::Handler;
use connection::Connection;



pub trait Factory {
    type Handler: Handler;

    fn on_accept(&mut self, token: Token) -> Self::Handler;
    fn on_hup(&mut self) {
        debug!("factory -> on-hup;")
    }
}

impl<F, H> Factory for F
    where H: Handler,
          F: FnMut(Token) -> H
{
    type Handler = H;

    fn on_accept(&mut self, token: Token) -> H { self(token) }
}
