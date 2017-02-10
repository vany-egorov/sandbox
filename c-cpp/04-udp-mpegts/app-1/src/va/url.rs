extern crate libc;

use self::libc::c_char;


#[repr(C)]
pub struct URL {
    pub scheme: u32,
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
