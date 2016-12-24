use mio::Token;
use mio::tcp::TcpStream;
use http;


#[derive(Debug, Copy, Clone, PartialEq)]
pub enum State {
    Accepted,     // TCP connection accepted
    TCP,          // Raw TCP client
    HTTPRq,       // Got HTTP request
    HTTPRs,       // Send HTTP response
    WS,           // Got HTTP Request, switched to Websocket. WebSocket connection.
    Disconnected, // client disconnected
}


pub struct Client {
    pub token: Token,
    pub conn: TcpStream,

    pub http_request: http::Request,

    pub state: State,
}

impl Client {
    pub fn new(token: Token, conn: TcpStream) -> Client {
        Client {
            token: token,
            conn: conn,

            http_request: http::Request::new(),

            state: State::Accepted,
        }
    }

    pub fn http_request_read(&mut self) -> Result<(), http::RequestError> {
        self.http_request.decode_from(&mut self.conn)
    }

    pub fn http_request_reset(&mut self) { self.http_request = http::Request::new(); }
}
