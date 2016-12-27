pub trait HandlerTCP {
    fn on_tcp_accept(&mut self) { /* */ };
    fn on_tcp_read(&mut self);
    fn on_tcp_hup(&mut self) { /* */ };
}

impl<H> HandlerTCP for H
    where H: FnMut()
{
    fn on_tcp_read(&mut self) {
        self()
    }
}
