use std::io::Write;

use http::Request;
use http::Response;


pub trait HandlerHTTP {
    fn on_tcp_accept(&mut self) { /* */ }
    fn on_tcp_read(&mut self) { /* */ }

    fn on_http_request(&mut self, _: &Request) { /* */ }
    fn on_http_response(&mut self, id: u64, req: &Request, resp: &mut Response, w: &mut Write);
    fn on_http_response_after(&mut self, _: u64, _: &Request, _: &Response) { /* */ }

    fn on_tcp_hup(&mut self) { /* */ }
}

impl<F> HandlerHTTP for F
    where F: FnMut(u64, &Request, &mut Response, &mut Write)
{
    fn on_http_response(&mut self, id: u64, req: &Request, resp: &mut Response, w: &mut Write) {
        self(id, req, resp, w)
    }
}
