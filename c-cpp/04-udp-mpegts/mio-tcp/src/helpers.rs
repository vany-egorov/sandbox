use std::fmt;
use std::usize;
use std::net::{
    SocketAddr,
    ToSocketAddrs
};

use mio::Token;

use result::Result;
use server_builder::ServerBuilder;
use handler::Handler;
use connection::Connection;


pub fn listen<A, F, H>(addr_spec: A, factory: F) -> Result<()>
    where
        A: ToSocketAddrs + fmt::Debug,
        F: FnMut(Token) -> H,
        H: Handler,
{
    let addr = try!(addr_spec.to_socket_addrs())
        .nth(0)    // first
        .unwrap(); // TODO: handle error
    let mut server = try!(ServerBuilder::new().finalize(factory));
    try!(server.listen_and_serve(&addr));

    Ok(())
}
