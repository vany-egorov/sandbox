extern crate regex;

use std;
use std::fmt;
use std::io::{
      Read
    , BufReader
    , BufRead
};
use self::regex::Regex;

use http::method;
use http::header;


const CR: u8 = 0x0D;
const LF: u8 = 0x0A;


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


#[derive(Debug)]
pub enum HeaderError {
}


#[derive(Default)]
pub struct Request {
    pub method: method::Method,
    pub url_raw: String,

    // HTTP/1.0
    pub proto_major: u8, // 1
    pub proto_minor: u8, // 0

    pub header: header::Header,
    pub content_length: usize,

    pub body: Vec<u8>,
}

impl Request {
    pub fn new() -> Request {
        Request {
            method: method::Method::GET,
            url_raw: "".to_string(),
            proto_major: 1,
            proto_minor: 0,
            header: header::Header::new(),
            content_length: 0,
            body: Vec::new(),
        }
    }

    pub fn decode_from(&mut self, src: &mut Read) {
        let mut i = -1;
        let mut buf = Vec::new();
        let mut reader = BufReader::new(src);
        loop {
            i += 1;
            buf.clear();

            match reader.read_until(LF, &mut buf) {
                Err(e) => {
                    println!("read header error: {}", e);
                    break;
                },
                Ok(len) => {
                    let s = std::str::from_utf8(&buf).unwrap();
                    if len == 2 && buf == [CR, LF] {
                        break;
                    }

                    if i == 0 {
                        let caps = RE_REQUEST_LINE.captures(s).unwrap();
                        self.method = method::Method::from(caps.name("method").unwrap());
                        self.url_raw = caps.name("url_raw").unwrap().to_string();
                        self.proto_major = caps.name("proto_major").unwrap().parse::<u8>().unwrap();
                        self.proto_minor = caps.name("proto_minor").unwrap().parse::<u8>().unwrap();
                    } else {
                        match RE_HEADER.captures(s) {
                            Some(caps) => {
                                let k = caps.name("k").unwrap().to_string();
                                let v = caps.name("v").unwrap().to_string();
                                if k == "Content-Length" {
                                    self.content_length = v.parse::<usize>().unwrap();
                                }
                                self.header.add(k, v);
                            },
                            None => {},
                        }
                    }
                }
            }
        }

        if self.content_length != 0 {
            self.body = Vec::with_capacity(self.content_length);
            match reader.read_to_end(&mut self.body) {
                Err(e) => {
                    match e.kind() {
                        std::io::ErrorKind::WouldBlock => {},
                        _ => {}
                    }
                },
                Ok(len) => {
                }
            }
        }
    }
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

        if self.content_length != 0 {
            try!(write!(f, "{}", std::str::from_utf8(&self.body).unwrap()));
        }

        Ok(())
    }
}
