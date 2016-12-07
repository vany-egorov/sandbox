extern crate libc;
use libc::{size_t, c_char};


#[repr(C)]
struct VAParser {
}

#[repr(C)]
struct VAParserOpenArgs {
}


#[link(name = "va", kind = "static")]
extern {
    fn va_parser_new(it: *mut VAParser);
    fn va_parser_open(it: *mut VAParser, args: *const VAParserOpenArgs);
    fn va_parser_go(it: *mut VAParser);
    fn va_parser_del1(it: *mut VAParser);
}


fn main() {
    println!("Hello, world!");
}
