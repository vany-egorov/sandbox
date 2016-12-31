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
    HandlerWS,
    HTTPRequest,
    HTTPResponse,
    Result as MIOTCPResult
};
use mio_tcp::http;


struct RootHandler {}

impl HandlerHTTP for RootHandler {
    fn on_http_response(&mut self, _: u64, req: &HTTPRequest, resp: &mut HTTPResponse, w: &mut Write) -> MIOTCPResult<()> {
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
            _ => { resp.set_status(http::Status::NotFound); }
        }

        Ok(())
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


struct WSHandler { }

impl HandlerWS for WSHandler {
    fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        println!("[#{}] {} {:5} {:15} [<- {}+{}] [-> {}]",
            id,
            resp.status as u32, req.method, req.url_raw,
            req.header_length, req.content_length,
            resp.content_length(),
        );
    }
}


fn route(req: &HTTPRequest) -> Handler {
    match req.path().as_ref() {
        "/ws/v1" => Handler::WS(Box::new(WSHandler{})),
        _        => Handler::HTTP(Box::new(RootHandler{})),
    }
}


fn main() {
    env_logger::init().unwrap();

    match listen("0.0.0.0:8000", route) {
        Err(e) => println!("error => {}", e),
        Ok(..) => {},
    }
}
