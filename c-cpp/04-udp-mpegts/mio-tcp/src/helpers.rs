use std::fmt;
use std::net::ToSocketAddrs;

use http::Request;
use result::Result;
use server_builder::ServerBuilder;
use handler::Handler;


pub fn listen<A, R>(addr_spec: A, router: R) -> Result<()>
    where
        A: ToSocketAddrs + fmt::Debug,
        R: FnMut(&Request) -> Handler
{
    let addr = try!(addr_spec.to_socket_addrs())
        .nth(0)    // first
        .unwrap(); // TODO: handle error
    let mut server = try!(ServerBuilder::new().finalize(router));
    try!(server.listen_and_serve(&addr));

    Ok(())
}
