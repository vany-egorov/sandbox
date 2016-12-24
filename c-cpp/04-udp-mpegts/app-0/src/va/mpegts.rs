extern crate libc;

use libc::c_void;


#[repr(C)]
pub struct MPEGTS {
    pub psi_pat: *mut MPEGTSPSIPAT,
    pub psi_pmt: *mut MPEGTSPSIPMT,
    pub psi_sdt: *mut MPEGTSPSISDT,
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
