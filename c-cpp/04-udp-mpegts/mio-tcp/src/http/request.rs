extern crate regex;

use std;
use std::fmt;
use std::io::{
      Read
    , BufReader
    , BufRead
};
use self::regex::Regex;

use http::{
    CR,
    LF,
    HEADER_CONTENT_LENGTH,
};
use http::method;
use http::header;
use http::request_error::{RequestError, RequestResult};


lazy_static! {
    static ref RE_REQUEST_LINE: Regex = Regex::new(r"(?x)
        ^
            (?P<method>GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT|PATCH|OPTIONS)
        \s
            (?P<url_raw>.+)
        \s
            HTTP/(?P<proto_major>\d)\.(?P<proto_minor>\d)
        (?:\r\n)?
        $").unwrap();
    static ref RE_HEADER: Regex = Regex::new(r"(?x)
        ^
            (?P<k>.+)
        :\s
            (?P<v>.+)
        (?:\r\n)?
        $").unwrap();
}


#[derive(Default)]
pub struct Request {
    pub method: method::Method,
    pub url_raw: String,

    // HTTP/1.0
    pub proto_major: u8, // 1
    pub proto_minor: u8, // 0

    pub header: header::Header,
    pub header_length: usize,
    pub content_length: usize,
}

impl Request {
    pub fn new() -> Request {
        Request {
            method: method::Method::GET,
            url_raw: "".to_string(),
            proto_major: 1,
            proto_minor: 0,
            header: header::Header::new(),
            header_length: 0,
            content_length: 0,
        }
    }

    pub fn decode_from<S>(src: &mut S) -> RequestResult<Option<Request>>
            where S: Read {
        let mut it = Request::new();

        let mut i = -1;
        let mut buf = Vec::with_capacity(128);
        let mut reader = BufReader::new(src);
        loop {
            i += 1;
            buf.clear();

            match reader.read_until(LF, &mut buf) {
                Err(err) => return Err(RequestError::from(err)),
                Ok(len) => {
                    it.header_length += len;
                    let s = try!(std::str::from_utf8(&buf));

                    if i == 0 && s == "" { return Ok(None); }

                    if len == 2 && buf == [CR, LF] {
                        break;
                    }

                    if i == 0 {
                        let caps = match RE_REQUEST_LINE.captures(s) {
                            Some(c) => c,
                            None => return Err(RequestError::RequestLineMissing),
                        };
                        it.method = method::Method::from(caps.name("method").unwrap());
                        it.url_raw = caps.name("url_raw").unwrap().to_string();
                        it.proto_major = try!(caps.name("proto_major").unwrap().parse::<u8>());
                        it.proto_minor = try!(caps.name("proto_minor").unwrap().parse::<u8>());
                    } else {
                        match RE_HEADER.captures(s) {
                            Some(caps) => {
                                let k = caps.name("k").unwrap().to_string();
                                let v = caps.name("v").unwrap().to_string();
                                if k == HEADER_CONTENT_LENGTH {
                                    it.content_length = try!(v.parse::<usize>());
                                }
                                it.header.add(k, v);
                            },
                            None => {},
                        }
                    }
                }
            }
        }

        Ok(Some(it))
    }

    pub fn path(&self) -> &String { &self.url_raw }
    pub fn method(&self) -> method::Method { self.method }
}

impl fmt::Display for Request {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        try!(write!(f, "{method} {url_raw} HTTP/{proto_major}.{proto_minor}\n",
            method=self.method,
            url_raw=self.url_raw,
            proto_major=self.proto_major,
            proto_minor=self.proto_minor
        ));

        for (k, vs) in &self.header {
            for v in vs {
                try!(write!(f, "{k}: {v}\n", k=k, v=v));
            }
        }

        Ok(())
    }
}
