extern crate mio_tcp;
extern crate env_logger;


use mio_tcp::listen;
use mio_tcp::Handler;
use mio_tcp::HTTPRequest;
use mio_tcp::HTTPResponse;
use mio_tcp::HandlerHTTP;


struct DefaultHandler {
}

impl HandlerHTTP for DefaultHandler {
    fn on_http_response(&mut self, req: &HTTPRequest, _: &mut HTTPResponse) {
        println!("~~~~~~~~~~~~~~~~~~~");
        print!("{}", req);
        println!("~~~~~~~~~~~~~~~~~~~");
    }
}


fn main() {
    env_logger::init().unwrap();

    match listen("0.0.0.0:8000", |token| {
        println!("someone connected: {:?}", token);

        Handler::HTTP(Box::new(DefaultHandler{}))

        // Some(HTTP(|| {

        // }))

        // Some(WS(|| {

        // }))
    }) {
        Err(e) => println!("error => {}", e),
        Ok(..) => {},
    }
}
