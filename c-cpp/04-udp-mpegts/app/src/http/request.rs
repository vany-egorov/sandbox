use http::method;


pub struct Request {
    pub method: method::Method,
}

impl Request {
    pub fn new() -> Request {
        Request {
            method: method::Method::GET,
        }
    }
}

impl Default for Request {
    fn default() -> Request {
        Request::new()
    }
}
