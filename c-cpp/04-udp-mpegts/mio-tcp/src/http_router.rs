use http::Request;
use handler::Handler;


pub trait HTTPRouter {
    fn on_tcp_accept(&mut self) { /* */ }
    fn on_tcp_read(&mut self) { /* */ }

    fn route(&self, req: &Request) -> Handler;
}
