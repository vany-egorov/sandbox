// use handler_tcp::HandlerTCP;


// pub enum Handler {
//     TCP(Box<HandlerTCP>),
//     HTTP(Box<HTTP>),
//     WS(Box<WS>),
// }

pub trait Handler {
    fn on_do(&mut self);
}

impl<H> Handler for H
    where H: FnMut()
{
    fn on_do(&mut self) {
        self()
    }
}
