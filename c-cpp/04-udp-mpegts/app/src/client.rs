use mio::Token;
use mio::tcp::TcpStream;
use http;


pub struct Client {
    pub token: Token,
    pub conn: TcpStream,

    pub http_request: http::Request,
}

impl Client {
    pub fn new(token: Token, conn: TcpStream) -> Client {
        Client {
            token: token,
            conn: conn,
            http_request: http::Request::new(),
        }
    }

    pub fn http_request_read(&mut self) -> Result<(), http::RequestError> {
        self.http_request.decode_from(&mut self.conn)
    }

    pub fn http_request_reset(&mut self) { self.http_request = http::Request::new(); }
}
