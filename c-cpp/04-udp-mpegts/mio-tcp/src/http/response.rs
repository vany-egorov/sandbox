use std::string::ToString;

use result::Result;
use http::{
    CCR, CLF,

    HEADER_CONTENT_LENGTH,
    HEADER_UPGRADE,
    HEADER_CONNECTION,
    HEADER_SEC_WEBSOCKET_ACCEPT,

    HEADER_V_WEBSOCKET,
    HEADER_V_UPGRADE,

    header,
    status,
    Request
};
use ws::{gen_key as ws_gen_key};


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
    pub fn header_set<S>(&mut self, k: S, v: S) -> &mut Response
        where S: Into<String>
    {
        self.header.set(k.into(), v.into());
        self
    }

    pub fn set_status(&mut self, v: status::Status) -> &mut Response {
        self.status = v;
        self
    }

    pub fn ws_upgrade(&mut self, req: &Request) -> Result<()> {
        // TODO: error handling
        let response_key = ws_gen_key(req.header.get_first("Sec-WebSocket-Key").unwrap().as_ref());

        self
            .set_status(status::Status::SwitchingProtocols)
            .header_set(HEADER_UPGRADE, HEADER_V_WEBSOCKET)
            .header_set(HEADER_CONNECTION, HEADER_V_UPGRADE)
            .header_set(HEADER_SEC_WEBSOCKET_ACCEPT, response_key.as_ref());

        Ok(())
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
