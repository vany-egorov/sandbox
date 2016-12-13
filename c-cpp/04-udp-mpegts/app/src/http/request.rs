pub enum Method {
    GET,
    POST,
    PUT,
    DELETE,
}

pub struct Request {
    pub method: Method,
}


impl Request {
    pub fn new() -> Request {
        Request {
            method: Method::GET,
        }
    }
}

impl Default for Request {
    fn default() -> Request {
        Request::new()
    }
}
