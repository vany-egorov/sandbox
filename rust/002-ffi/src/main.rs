extern crate libc;
use libc::c_char;
use libc::size_t;
use std::ffi::CString;


#[repr(C)]
pub enum URLScheme {
    UrlSchemeUdp = 1,
    UrlSchemeRtmp,
    UrlSchemeHttp,
    UrlSchemeHttps,
    UrlSchemeWs,
    UrlSchemeWss,
    UrlSchemeRtp,
    UrlSchemeFile,
    UrlSchemeSsh,

    UrlSchemeUnknown,
}


#[repr(C)]
pub struct URL {
    pub scheme: URLScheme,
    pub userinfo: [c_char; 100],
    pub host: [c_char; 100],
    pub port: u32,
    pub path: [c_char; 255],
    pub query: [c_char; 255],
    pub fragment: [c_char; 100],
    pub flags: u32,
}

#[link(name = "url", kind = "static")]
extern {
    fn url_parse(it: *mut URL, raw: *const c_char);
    fn url_sprint_json(it: *mut URL, buf: &[u8], bufsz: size_t);
}


fn main() {
    let mut url = URL{
        scheme : URLScheme::UrlSchemeWss,
        userinfo : [0; 100],
        host : [0; 100],
        port : 0,
        path : [0; 255],
        query: [0; 255],
        fragment: [0; 100],
        flags: 0,
    };
    let u = &mut url;

    let raw = CString::new("http://8.8.8.8:1748/google/com?foo=bar&bar=buz#section1").unwrap();

    let json: [u8; 255] = [0; 255];

    unsafe {
        url_parse(u, raw.as_ptr());
        url_sprint_json(u, &json, 255);
    }

    let s = String::from_utf8(json.to_vec()).unwrap();
    println!("{:?}", s);
}
