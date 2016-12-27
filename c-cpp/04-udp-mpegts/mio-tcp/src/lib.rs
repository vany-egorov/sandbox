extern crate mio;
extern crate slab;
#[macro_use] extern crate log; /* error! */
#[macro_use] extern crate lazy_static; /* lazy_static! */

mod error;
mod result;
mod helpers;
mod server;
mod server_builder;
mod server_settings;
mod connection;
mod tokens;
mod factory;
mod handler;
mod handler_tcp;

mod http;
mod ws;


pub use helpers::listen;
pub use handler::Handler;
pub use handler_tcp::HandlerTCP;
pub use http::{
    Handler as HandlerHTTP,
    Request as HTTPRequest,
    Response as HTTPResponse,
};
pub use ws::Handler as HandlerWS;
