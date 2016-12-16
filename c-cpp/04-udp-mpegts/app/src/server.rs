use std::collections::HashMap;

use mio::Token;
use mio::tcp::{TcpListener, TcpStream};


pub struct Server {
    pub clients: HashMap<Token, TcpStream>,
}


impl Server {
    pub fn new() -> Server {
        Server {
            clients: HashMap::new(),
        }
    }
}
