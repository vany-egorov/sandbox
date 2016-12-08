extern crate libc;
#[macro_use]
extern crate chan;
extern crate chan_signal;

use libc::{sockaddr_in};
use libc::{size_t, sem_t, pthread_t, pthread_mutex_t};
use libc::{c_char, c_int, c_uint, c_void};

use chan_signal::Signal;


#[repr(C)]
pub struct H264 {
    pub _1_: u64,
    pub _2_: u64,
    pub _3_: u64,
    pub _4_: u64,
    pub _5_: u64,
    pub _6_: u64,
}

#[repr(C)]
pub struct MPEGTSPSI {
    pub _1_: u8,
    pub _2_: u8,
    pub _3_: u8,
    pub _4_: u16,
    pub _5_: u16,
    pub _6_: u8,
    pub _7_: u8,
    pub _8_: u8,
    pub _9_: u32,
}

pub struct MPEGTSPSIPAT {
    pub psi: MPEGTSPSI,
    pub program_number: u16,
    pub program_map_pid: u16,
}

#[repr(C)]
pub struct MPEGTSPSIPMT {
    pub psi: MPEGTSPSI,
    pub pcr_pid: u16,
    pub program_info_length: u16,
    pub program_elements: MPEGTSPSIPMTProgramElements,
}

#[repr(C)]
pub struct MPEGTSPSIPMTProgramElements {
    pub len: u8,
    pub cap: u8,
    pub c: *mut c_void,
}

#[repr(C)]
pub struct MPEGTSPSISDT {
    pub psi: MPEGTSPSI,
    pub original_network_id: u16,
    pub reserved_future_use: u8,
    pub services: MPEGTSPSISDTServices,
}

pub struct MPEGTSPSISDTServices {
    pub len: u8,
    pub cap: u8,
    pub c: *mut c_void,
}

#[repr(C)]
pub struct MPEGTS {
    pub psi_pat: *mut MPEGTSPSIPAT,
    pub psi_pmt: *mut MPEGTSPSIPMT,
    pub psi_sdt: *mut MPEGTSPSISDT,
}

#[repr(C)]
pub struct IOReader {
    pub w: *mut c_void,
    pub read: IOReaderReadFunc,
}

pub type IOReaderReadFunc =
    ::std::option::Option<unsafe extern "C" fn(
        ctx: *mut c_void,
        buf: *mut u8,
        bufsz: size_t,
        n: *mut size_t
    ) -> c_int>;

#[repr(C)]
pub struct IOMultiWriter {
    pub writers: *mut *mut IOWriter,
    pub len: size_t,
}

pub type IOWriterWriteFunc =
    ::std::option::Option<unsafe extern "C" fn(
        ctx: *mut c_void,
        buf: *mut u8,
        bufsz: size_t,
        n: *mut size_t
    ) -> c_int>;

#[repr(C)]
pub struct IOWriter {
    pub w: *mut c_void,
    pub write: IOWriterWriteFunc,
}

#[repr(C)]
pub struct FIFO {
    pub data: *mut u8,
    pub len: size_t,
    pub cap: size_t,
    pub start: size_t,
    pub finish: size_t,
    pub sem: sem_t,
    pub rw_mutex: pthread_mutex_t,
}

#[repr(C)]
pub struct UDP {
    pub sock: c_int,
    pub addrlen: c_uint,
    pub addr: sockaddr_in,
}

#[repr(i32)]
pub enum URLScheme {
    UrlSchemeUdp = 0,
    UrlSchemeRtmp = 1,
    UrlSchemeHttp = 2,
    UrlSchemeHttps = 3,
    UrlSchemeWs = 4,
    UrlSchemeWss = 5,
    UrlSchemeRtp = 6,
    UrlSchemeFile = 7,
    UrlSchemeSsh = 8,
    UrlSchemeUnknown = 9,
}

#[repr(C)]
pub struct URL {
    pub scheme: URLScheme,
    pub port: u16,
    pub buf: [c_char; 255usize],
    pub buf_len: u16,
    pub _bindgen_bitfield_1_: u8,
    pub _bindgen_bitfield_2_: u8,
    pub _bindgen_bitfield_3_: u8,
    pub _bindgen_bitfield_4_: u8,
    pub _bindgen_bitfield_5_: u8,
    pub _bindgen_bitfield_6_: u8,
    pub pos_userinfo: u16,
    pub pos_host: u16,
    pub pos_path: u16,
    pub pos_query: u16,
    pub pos_fragment: u16,
    pub flags: u8,
}

#[repr(C)]
pub struct VAParserWorkerRead {
    pub reader: *mut IOReader,
    pub writer: *mut IOWriter,
    pub thread: pthread_t,
}

#[repr(C)]
pub struct VAParserWorkerParse {
    pub fifo: *mut FIFO,
    pub mpegts: MPEGTS,
    pub h264: H264,
    pub cb: VAParserParseCBFunc,
    pub offset: u64,
    pub video_pid_h264: u16,
    pub thread: pthread_t,
}

#[repr(C)]
pub struct VAParser {
    pub i: URL,
    pub udp: *mut UDP,
    pub fifo: *mut FIFO,
    pub multi: *mut IOMultiWriter,
    pub reader_udp: *mut IOReader,
    pub reader_fifo: *mut IOReader,
    pub writer_fifo: *mut IOWriter,
    pub writer_multi: *mut IOWriter,
    pub worker_read: VAParserWorkerRead,
    pub worker_parse: VAParserWorkerParse,
}

impl Default for VAParser {
    fn default() -> Self { unsafe { ::std::mem::zeroed() } }
}

pub type VAParserParseCBFunc = ::std::option::Option<unsafe extern "C" fn(ctx: *mut c_void) -> c_int>;

#[repr(C)]
pub struct VAParserOpenArgs {
    pub i_url_raw: *const c_char,
    pub cb: VAParserParseCBFunc,
}

impl Default for VAParserOpenArgs {
    fn default() -> Self { unsafe { ::std::mem::zeroed() } }
}

#[link(name = "va", kind = "static")]
extern {
    fn va_parser_open(it: *mut VAParser, args: *const VAParserOpenArgs);
    fn va_parser_go(it: *mut VAParser);
}


unsafe extern "C" fn va_parser_parse_cb(ctx: *mut c_void) -> c_int {
    println!("I'm called from C in Rust!");
    return 0;
}

fn main() {
    let signal = chan_signal::notify(&[Signal::INT, Signal::TERM]);
    let raw = std::ffi::CString::new("udp://239.1.1.1:5500").unwrap();

    let mut va_parser: VAParser = Default::default();
    let va_parser_open_args = VAParserOpenArgs{
        i_url_raw : raw.as_ptr(),
        cb : Some(va_parser_parse_cb),
    };

    unsafe {
        va_parser_open(&mut va_parser, &va_parser_open_args);
        va_parser_go(&mut va_parser);
    }

    chan_select! {
        signal.recv() -> signal => {
            println!("received signal: {:?}", signal)
        }
    }
}
