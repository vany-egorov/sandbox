use mio::Token;

use handler::Handler;
use connection::Connection;



pub trait Factory {
    type Handler: Handler;

    fn produce(&mut self, token: Token) -> Self::Handler;
}

impl<F, H> Factory for F
    where H: Handler,
          F: FnMut(Token) -> H
{
    type Handler = H;

    fn produce(&mut self, token: Token) -> H { self(token) }
}
