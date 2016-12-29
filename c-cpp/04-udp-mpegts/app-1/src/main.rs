extern crate mio_tcp;
extern crate env_logger;

use std::io::{
    Write,
    copy
};
use std::fs::File;

use mio_tcp::{
    listen,
    Handler,
    HandlerHTTP,
    HTTPRequest,
    HTTPResponse,
    HTTPRouter,
};
use mio_tcp::http;


struct IndexHandler {}

impl HandlerHTTP for IndexHandler {
    fn on_http_response(&mut self, _: u64, req: &HTTPRequest, resp: &mut HTTPResponse, w: &mut Write) {
        match req.path().as_ref() {
            "/" => {
                copy(&mut File::open("./static/index.html").unwrap(), w).unwrap();

                resp.header_set("Content-Type",
                    "text/html; charset=UTF-8");
            },
            "/static/app.css" => {
                copy(&mut File::open("./static/app.css").unwrap(), w).unwrap();

                resp.header_set("Content-Type",
                    "text/css; charset=UTF-8");
            },
            "/static/app.js" => {
                copy(&mut File::open("./static/app.js").unwrap(), w).unwrap();

                resp.header_set("Content-Type",
                    "text/javascript; charset=UTF-8");
            },
            "/ws/v1" => { resp.set_status(http::Status::NotFound); },
            _ => { resp.set_status(http::Status::NotFound); }
        }
    }

    fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        println!("[#{}] {} {:5} {:15} [<- {}+{}] [-> {}]",
            id,
            resp.status as u32, req.method, req.url_raw,
            req.header_length, req.content_length,
            resp.content_length(),
        );
    }
}


struct Router {}

impl HTTPRouter for Router {
    fn route(&self, req: &HTTPRequest) -> Handler {
        Handler::HTTP(Box::new(IndexHandler{}))
    }
}


fn main() {
    env_logger::init().unwrap();

    match listen("0.0.0.0:8000", |_| {
        // println!("[#{}] [+]", usize::from(token));

        Handler::HTTPRouter(Box::new(Router{}))
    }) {
        Err(e) => println!("error => {}", e),
        Ok(..) => {},
    }
}
