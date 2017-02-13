extern crate libc;
extern crate mio_tcp;
extern crate env_logger;
extern crate rustc_serialize;           // MessagePack
extern crate rmp_serialize as msgpack;  // MessagePack

use std::io::{
    Write,
    copy
};
use std::net::SocketAddr;
use std::fs::File;
use std::path::Path;

use mio_tcp::{
    http,
    listen,
    Handler,
    HandlerHTTP,
    HandlerWS,
    HTTPRequest,
    HTTPResponse,
    Result as MIOTCPResult,
    Message, MessageKind, MessageBody,
    ServerBuilder,
    ChannelSyncSender,

};
use libc::{c_int, c_void};
use msgpack::{Encoder, Decoder};
use rustc_serialize::Encodable;

mod va;


fn log_req_res(id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
    println!("[#{}] {} {:5} {:15} [<- {}+{}] [-> {}]",
        id,
        resp.status as u32, req.method, req.url_raw,
        req.header_length, req.content_length,
        resp.content_length(),
    );
}


struct RootHandler {}

impl HandlerHTTP for RootHandler {
    fn on_http_response(&mut self, _: u64, _: &HTTPRequest, resp: &mut HTTPResponse, w: &mut Write) -> MIOTCPResult<()> {
        let path = Path::new("./static/index.html");

        if !path.exists() {
            resp.set_status(http::Status::NotFound);
            return Ok(());
        }

        let mut r = try!(File::open(path));
        try!(copy(&mut r, w));

        resp.header_set("Content-Type",
            "text/html; charset=UTF-8");

        Ok(())
    }

    fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        log_req_res(id, req, resp);
    }
}

struct StaticDirHandler { }

impl HandlerHTTP for StaticDirHandler {
    fn on_http_response(&mut self, _: u64, req: &HTTPRequest, resp: &mut HTTPResponse, w: &mut Write) -> MIOTCPResult<()> {
        let mut p: String = ".".to_string(); // root directory
        p.push_str(req.path());

        let path = Path::new(&p);

        if !path.exists() {
            resp.set_status(http::Status::NotFound);
            return Ok(());
        }

        let content_type = match path.extension() {
            Some(ext) => {
                if ext == "css" {
                    "text/css; charset=UTF-8"
                } else if ext == "js" {
                    "text/javascript; charset=UTF-8"
                } else if ext == "html" {
                    "text/html; charset=UTF-8"
                } else if ext == "json" {
                    "application/json; charset=UTF-8"
                } else if ext == "xml" {
                    "application/xml; charset=UTF-8"
                } else {
                    "text/plain; charset=UTF-8"
                }
            },
            _  => "text/plain; charset=UTF-8",
        };

        resp.header_set("Content-Type", content_type);

        let mut r = try!(File::open(path));
        try!(copy(&mut r, w));

        Ok(())
    }

    fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        log_req_res(id, req, resp);
    }
}

struct WSHandler { }

impl HandlerWS for WSHandler {
    fn on_ws_message(&mut self, id: u64, size: usize) -> MIOTCPResult<()> {
        println!("[#{}] [ws] [<] {}", id, size);

        Ok(())
    }

    fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        log_req_res(id, req, resp);
    }
}

struct NotFoundHandler { }

impl HandlerHTTP for NotFoundHandler {
    fn on_http_response(&mut self, _: u64, _: &HTTPRequest, resp: &mut HTTPResponse, _: &mut Write) -> MIOTCPResult<()> {
        resp.set_status(http::Status::NotFound);

        Ok(())
    }

    fn on_http_response_after(&mut self, id: u64, req: &HTTPRequest, resp: &HTTPResponse) {
        log_req_res(id, req, resp);
    }
}


fn route(req: &HTTPRequest) -> Handler {
    let path = req.path().as_ref();

    match path {
        "/"                              => Handler::HTTP(Box::new(RootHandler{})),
        "/ws/v1"                         => Handler::WS(Box::new(WSHandler{})),
        _ if path.starts_with("/static") => Handler::HTTP(Box::new(StaticDirHandler{})),
        _                                => Handler::HTTP(Box::new(NotFoundHandler{})),
    }
}


pub struct CbCtx {
    tx: ChannelSyncSender<Message>,
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

    let cb_ctx: &mut CbCtx = unsafe { &mut *(ctx as *mut CbCtx) };

    if atom_kind == va::AtomKind::H264SliceIDR {
        let h264_slice_idr: &mut va::h264::H264NALSliceIDR = unsafe { &mut *(atom as *mut va::h264::H264NALSliceIDR) };
        // println!("0x{:08X} {:?} {:?} {} {}", offset, h264_slice_idr.nt, h264_slice_idr.st, h264_slice_idr.frame_num, h264_slice_idr.pic_order_cnt_lsb);
        let mut encoded = Vec::new();
        {
            let mut encoder = Encoder::new(&mut encoded);
            h264_slice_idr.encode(&mut encoder);
        }
        cb_ctx.tx.send(Message{kind: MessageKind::WsBroadcast, body: MessageBody::Bin(encoded)});
    }
    // println!("0x{:08X} | {:p} | {:p} | {:?}", offset, ctx, atom, atom_kind);
    return 0;
}

fn main() {
    env_logger::init().unwrap();

    let addr: SocketAddr = match "0.0.0.0:8000".parse() {
        Ok(v) => v,
        Err(e) => {
            println!("parse-addr failed: {:?}", e);
            std::process::exit(1);
        }
    };

    let mut server = match ServerBuilder::new().finalize(route) {
        Ok(v) => v,
        Err(e) => {
            println!("create server instance failed: {:?}", e);
            std::process::exit(1);
        }
    };
    let tx = server.tx();

    let mut va_parser: va::Parser = Default::default();
    let mut cb_ctx = CbCtx{tx: tx};
    let cb_ctx_ptr: *mut c_void = &mut cb_ctx as *mut _ as *mut c_void;
    let raw = std::ffi::CString::new("udp://239.1.1.1:5500").unwrap();
    let va_parser_open_args = va::ParserOpenArgs{
        i_url_raw : raw.as_ptr(),
        cb : Some(va_parser_parse_cb),
        cb_ctx : cb_ctx_ptr,
    };

    unsafe {
        va::va_parser_open(&mut va_parser, &va_parser_open_args);
        va::va_parser_go(&mut va_parser);
    }
    server.listen_and_serve(&addr);

    // match listen("0.0.0.0:8000", route) {
    //     Err(e) => println!("error starting HTTP/WS server: {}", e),
    //     Ok(..) => {},
    // }
}
