extern crate mio_tcp;

use mio_tcp::listen;


fn main() {
    match listen("0.0.0.0:8000", || {

    }) {
        Err(e) => println!("error => {}", e),
        Ok(..) => {},
    }
}
