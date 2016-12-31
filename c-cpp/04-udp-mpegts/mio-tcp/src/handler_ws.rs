use std::io::Write;

use http::Request;
use http::Response;


pub trait HandlerWS {
    fn on_tcp_accept(&mut self) { /* */ }
    fn on_tcp_read(&mut self) { /* */ }

    fn on_http_request(&mut self, _: &Request) { /* */ }
    fn on_http_response(&mut self, id: u64, req: &Request, resp: &mut Response, w: &mut Write);
    fn on_http_response_after(&mut self, _: u64, _: &Request, _: &Response) { /* */ }

    fn on_tcp_hup(&mut self) { /* */ }
}
