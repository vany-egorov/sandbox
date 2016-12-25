use std::usize;

use mio::Token;


pub const SERVER: Token = Token(usize::MAX - 1);
pub const BUS: Token = Token(usize::MAX - 2);
