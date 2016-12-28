use std::string::ToString;

use http::{CCR, CLF, HEADER_CONTENT_LENGTH};
use http::header;
use http::status;


#[derive(Default)]
pub struct Response {
    pub status: status::Status,

    // HTTP/1.0
    pub proto_major: u8, // 1
    pub proto_minor: u8, // 0

    pub header: header::Header,
    content_length: usize,
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

impl Response {
    pub fn content_length(&self) -> usize { self.content_length }
    pub fn set_content_length(&mut self, v: usize) { self.content_length = v; }

    pub fn header_mut(&mut self) -> &mut header::Header { &mut self.header }
    pub fn header_set<S>(&mut self, k: S, v: S)
        where S: Into<String>
    {
        self.header.set(k.into(), v.into())
    }

    pub fn set_status(&mut self, v: status::Status) { self.status = v; }
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
            CR=CCR, LF=CLF
        ));

        for (k, vs) in &self.header {
            for v in vs {
                if v == HEADER_CONTENT_LENGTH {
                    continue
                }

                s.push_str(&format!("{k}: {v}{CR}{LF}", k=k, v=v, CR=CCR, LF=CLF));
            }
        }

        if self.content_length != 0 {
            s.push_str(&format!(
                "{k}: {v}{CR}{LF}",
                k=HEADER_CONTENT_LENGTH,
                v=self.content_length,
                CR=CCR,
                LF=CLF
            ));
        }

        s.push(CCR);
        s.push(CLF);

        s
    }
}
