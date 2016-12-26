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

