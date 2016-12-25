use std::fmt;
use std::net::{
    SocketAddr,
    ToSocketAddrs
};

use result::Result;
use server_builder::ServerBuilder;


pub fn listen<A, F>(addr_spec: A, factory: F) -> Result<()>
    where
        A: ToSocketAddrs + fmt::Debug,
        F: FnMut(),
{
    let addr = try!(addr_spec.to_socket_addrs())
        .nth(0)    // first
        .unwrap(); // TODO: handle error
    let mut server = try!(ServerBuilder::new().finalize());
    try!(server.listen_and_serve(&addr));

    Ok(())
}
