use std::string::ToString;

use http::{CR, LF};
use http::header;
use http::status;


#[derive(Default)]
pub struct Response {
    pub status: status::Status,

    // HTTP/1.0
    pub proto_major: u8, // 1
    pub proto_minor: u8, // 0

    pub header: header::Header,
    pub content_length: usize,
}

impl Response {
    pub fn new() -> Response {
        Response {
            status: status::Status::OK,

            proto_major: 1,
            proto_minor: 1,

            header: header::Header::new(),
            content_length: 0,
        }
    }
}

impl ToString for Response {
    fn to_string(&self) -> String {
        let mut s = String::new();

        s.push_str(&format!(
            "HTTP/{proto_major}.{proto_minor} {http_status_code} {http_status_text}{CR}{LF}",
            proto_major=self.proto_major,
            proto_minor=self.proto_minor,
            http_status_code=self.status as u16,
            http_status_text=self.status,
            CR=CR, LF=LF
        ));

        for (k, vs) in &self.header {
            for v in vs {
                s.push_str(&format!("{k}: {v}{CR}{LF}", k=k, v=v, CR=CR, LF=LF));
            }
        }

        s.push(CR as char);
        s.push(LF as char);

        s
    }
}
