pub trait Handler {
    fn on_http_request(&mut self) { };
    fn on_http_response(&mut self) { };
}

impl<H> Handler for H
    where H: FnMut()
{
    fn on_tcp_read(&mut self) {
        self()
    }
}

