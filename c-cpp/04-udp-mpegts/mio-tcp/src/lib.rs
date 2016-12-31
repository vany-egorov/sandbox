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
mod router;
mod handler;
mod handler_tcp;
mod handler_http;
mod handler_ws;

pub mod http;
pub mod ws;

pub use mio::Token;
pub use helpers::listen;
pub use handler::Handler;
pub use handler_tcp::HandlerTCP;
pub use handler_http::HandlerHTTP;
pub use handler_ws::HandlerWS;
pub use http::{
    Request as HTTPRequest,
    Response as HTTPResponse,
};
