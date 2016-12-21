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


pub struct CbCtx {
    tx: mio::channel::SyncSender<server::Command>,
}


unsafe extern "C" fn va_parser_parse_cb(ctx: *mut c_void, atom: *mut c_void, atom_kind: va::AtomKind, offset: u64) -> c_int {
    if atom_kind == va::AtomKind::MPEGTSHeader   ||
       atom_kind == va::AtomKind::MPEGTSAdaption ||
       atom_kind == va::AtomKind::MPEGTSPES      ||
       atom_kind == va::AtomKind::MPEGTSPSIPAT   ||
       atom_kind == va::AtomKind::MPEGTSPSIPMT   ||
       atom_kind == va::AtomKind::MPEGTSPSISDT {
        return 0;
    }

    if atom_kind == va::AtomKind::H264SliceIDR {
        let h264_slice_idr: &mut va::h264::H264NALSliceIDR = unsafe { &mut *(atom as *mut va::h264::H264NALSliceIDR) };
        println!("0x{:08X} {:?} {:?} {} {}", offset, h264_slice_idr.nt, h264_slice_idr.st, h264_slice_idr.frame_num, h264_slice_idr.pic_order_cnt_lsb);
    }

    let cb_ctx: &mut CbCtx = unsafe { &mut *(ctx as *mut CbCtx) };
    cb_ctx.tx.send(server::Command{token: mio::Token(535325), signal: server::Signal::A}).unwrap();
    // println!("0x{:08X} | {:p} | {:p} | {:?}", offset, ctx, atom, atom_kind);
    return 0;
}

fn main() {
    let signal = chan_signal::notify(&[Signal::INT, Signal::TERM]);
    let raw = std::ffi::CString::new(UDP_ADDR_RAW).unwrap();

    let mut srv = server::Server::new();
    let tx1 = srv.tx.clone();
    let tx2 = srv.tx.clone();
    std::thread::spawn(move || {
        match srv.listen_and_serve(ADDR_RAW) {
            Err(e) => {
                println!("listen-and-serve failed: {:?}", e);
                std::process::exit(1);
            }
            Ok(..) => {}
        }
    });

    std::thread::spawn(move || {
        loop {
            std::thread::sleep_ms(2000);
            tx1.send(server::Command{token: mio::Token(1213), signal: server::Signal::A}).unwrap();
        }
    });

    let mut va_parser: va::Parser = Default::default();
    let mut cb_ctx = CbCtx{tx: tx2};
    let cb_ctx_ptr: *mut c_void = &mut cb_ctx as *mut _ as *mut c_void;
    let va_parser_open_args = va::ParserOpenArgs{
        i_url_raw : raw.as_ptr(),
        cb : Some(va_parser_parse_cb),
        cb_ctx : cb_ctx_ptr,
    };

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
