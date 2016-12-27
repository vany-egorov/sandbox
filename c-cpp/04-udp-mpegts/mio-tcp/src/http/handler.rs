use http::Request;
use http::Response;


pub trait Handler {
    fn on_tcp_accept(&mut self) { /* */ }
    fn on_tcp_read(&mut self) { /* */ }

    fn on_http_request(&mut self) { /* */ }
    fn on_http_response(&mut self, req: &Request, resp: &mut Response);

    fn on_tcp_hup(&mut self) { /* */ }
}
