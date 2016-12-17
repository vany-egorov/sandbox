extern crate mio;
extern crate libc;
#[macro_use]
extern crate chan;
#[macro_use]
extern crate lazy_static;
extern crate chan_signal;

use libc::{c_int, c_void};
use chan_signal::Signal;

mod va;
mod http;
mod client;
mod server;


const ADDR_RAW: &'static str = "0.0.0.0:8000";
const UDP_ADDR_RAW: &'static str = "udp://239.1.1.1:5500";


unsafe extern "C" fn va_parser_parse_cb(ctx: *mut c_void, atom: *mut c_void, atom_kind: u32, offset: u64) -> c_int {
    if atom_kind == 0 { return 0; }
    println!("0x{:08X} | {:p} | {:p} | {}", offset, ctx, atom, atom_kind);
    return 0;
}

fn main() {
    let signal = chan_signal::notify(&[Signal::INT, Signal::TERM]);
    let raw = std::ffi::CString::new(UDP_ADDR_RAW).unwrap();

    let mut va_parser: va::Parser = Default::default();
    let va_parser_open_args = va::ParserOpenArgs{
        i_url_raw : raw.as_ptr(),
        cb : Some(va_parser_parse_cb),
    };

    let mut server = server::Server::new();
    std::thread::spawn(move || {
        match server.listen_and_serve(ADDR_RAW) {
            Err(e) => {
                println!("listen-and-serve failed: {:?}", e);
                std::process::exit(1);
            }
            Ok(..) => {}
        }
    });

    unsafe {
        va::va_parser_open(&mut va_parser, &va_parser_open_args);
        va::va_parser_go(&mut va_parser);
    }

    chan_select! {
        signal.recv() -> signal => {
            println!("received signal: {:?}", signal)
        }
    }
}
