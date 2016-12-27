extern crate mio_tcp;
extern crate env_logger;


use mio_tcp::listen;


fn main() {
    env_logger::init().unwrap();

    match listen("0.0.0.0:8000", |token| {
        println!("someone connected: {:?}", token);

        // Some(TCP(|| {

        // }))

        // Some(HTTP(|| {

        // }))

        // Some(WS(|| {

        // }))

        || {

        }
    }) {
        Err(e) => println!("error => {}", e),
        Ok(..) => {},
    }
}
