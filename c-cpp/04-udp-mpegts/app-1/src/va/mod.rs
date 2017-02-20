pub mod io;
pub mod url;

pub mod mpegts;
pub mod h264;

extern crate libc;

use self::libc::pthread_t;
use self::libc::{c_char, c_int, c_void};

#[repr(i32)]
#[derive(PartialEq, Debug)]
pub enum AtomKind {
    MPEGTSHeader,
    MPEGTSAdaption,
    MPEGTSPES,
    MPEGTSPSIPAT,
    MPEGTSPSIPMT,
    MPEGTSPSISDT,

    H264SPS,
    H264PPS,
    H264AUD,
    H264SEI,
    H264SliceIDR,
}

#[repr(C)]
pub struct ParserWorkerRead {
    pub reader: *mut io::IOReader,
    pub writer: *mut io::IOWriter,
    pub thread: pthread_t,
}

#[repr(C)]
pub struct ParserWorkerParse {
    pub fifo: *mut io::FIFO,
    pub mpegts: mpegts::MPEGTS,
    pub h264: h264::H264,
    pub cb_ctx: *mut c_void,
    pub cb: ParserParseCBFunc,
    pub offset: u64,
    pub video_pid_h264: u16,
    pub thread: pthread_t,
}

#[repr(C)]
pub struct Parser {
    pub i: url::URL,
    pub udp: *mut io::UDP,
    pub fifo: *mut io::FIFO,
    pub multi: *mut io::IOMultiWriter,
    pub reader_udp: *mut io::IOReader,
    pub reader_fifo: *mut io::IOReader,
    pub writer_fifo: *mut io::IOWriter,
    pub writer_multi: *mut io::IOWriter,
    pub worker_read: ParserWorkerRead,
    pub worker_parse: ParserWorkerParse,
}

#[repr(C)]
pub struct AtomWrapper {
    pub id: u64,
    pub offset: u64,
    pub kind: AtomKind,
    pub atom: *mut c_void,
}

impl Default for Parser {
    fn default() -> Self { unsafe { ::std::mem::zeroed() } }
}

pub type ParserParseCBFunc =
    ::std::option::Option<unsafe extern "C" fn(
        ctx: *mut c_void,
        aw: *mut AtomWrapper
    ) -> c_int>;

#[repr(C)]
pub struct ParserOpenArgs {
    pub i_url_raw: *const c_char,

    pub cb_ctx: *mut c_void,
    pub cb: ParserParseCBFunc,
}

impl Default for ParserOpenArgs {
    fn default() -> Self { unsafe { ::std::mem::zeroed() } }
}

#[link(name = "va", kind = "static")]
extern {
    pub fn va_parser_open(it: *mut Parser, args: *const ParserOpenArgs);
    pub fn va_parser_go(it: *mut Parser);
}
