use mio::{
    Poll,
    Token,
    Ready,
    PollOpt,
};
use mio::tcp::TcpStream;

use result::Result;
use handler::Handler;


pub struct Connection<H>
    where H: Handler
{
    pub token: Token,
    pub sock: TcpStream,
    pub handler: H
}


impl<H> Connection<H>
    where H: Handler
{
    pub fn new(token: Token, sock: TcpStream, handler: H) -> Connection<H> {
        Connection {
            token: token,
            sock: sock,
            handler: handler,
        }
    }
}
