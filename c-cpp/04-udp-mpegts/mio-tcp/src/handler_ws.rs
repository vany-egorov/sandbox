use std::io::Write;

use http::Request;
use http::Response;
use result::Result;


pub trait HandlerWS {
    fn on_tcp_accept(&mut self) { /* */ }
    fn on_tcp_read(&mut self) { /* */ }

    fn on_http_request(&mut self, _: &Request) { /* */ }
    fn on_http_response(&mut self, _: u64, req: &Request, resp: &mut Response, _: &mut Write) -> Result<()> {
        resp.ws_upgrade(req)
    }
    fn on_http_response_after(&mut self, _: u64, _: &Request, _: &Response) { /* */ }

    fn on_ws_message(&mut self, _: u64, _: usize) -> Result<()>;

    fn on_tcp_hup(&mut self) { /* */ }
}
