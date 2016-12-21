#[repr(C)]
pub struct H264 {
    pub _1_: u64,
    pub _2_: u64,
    pub _3_: u64,
    pub _4_: u64,
    pub _5_: u64,
    pub _6_: u64,
}


#[repr(i32)]
#[derive(PartialEq, Debug)]
pub enum H264NALType {
    Unspecified = 0,
    Slice       = 1,
    DPA         = 2,
    DPB         = 3,
    DPC         = 4,
    IDR         = 5,
    SEI         = 6,
    SPS         = 7,
    PPS         = 8,
    AUD         = 9,
    EOSEQ       = 10,
    EOSTREAM    = 11,
    FILL        = 12,
    SPSEXT      = 13,
    Prefix      = 14,
    SSPS        = 15,
    DPS         = 16,
    CSOACPWP    = 19,
    CSE         = 20,
    CSE3D       = 21,
}

#[repr(C)]
pub struct H264NAL { // 24 bytes
    pub t: H264NALType,
    pub data: [u32; 5usize], // union
}

#[repr(i32)]
#[derive(PartialEq, Debug)]
pub enum H264NALSliceType {
    P   = 0,
    B   = 1,
    I   = 2,
    SP  = 3,
    SI  = 4,
    P2  = 5,
    B2  = 6,
    I2  = 7,
    SP2 = 8,
    SI2 = 9,
}

#[repr(C)]
pub struct H264NALSliceIDR { // 4 bytes + 12 bytes = 16 bytes
    pub nt: H264NALType, // from H264NAL

    pub st: H264NALSliceType,
    pub bitfields: u8,
    pub pic_parameter_set_id: u16,
    pub frame_num: u16,
    pub pic_order_cnt_lsb: u16,
}

#[repr(C)]
pub struct H264NALSEI { // 4 + 1 = 8 byte
    pub nt: H264NALType, // from H264NAL

    pub reserved: u8,
}

#[repr(C)]
pub struct H264NALAUD { // 4 + 1 = 8 byte
    pub nt: H264NALType, // from H264NAL

    pub primary_pic_type: u8,
}

#[repr(C)]
pub struct H264NALSPS { // 4 + 18 = 24 bytes
    pub nt: H264NALType, // from H264NAL

    pub profile_idc: u8,
    pub bitfields1: u8,
    pub level_idc: u8,
    pub seq_parameter_set_id: u8,
    pub chroma_format_idc: u8,
    pub bitfields2: u8,
    pub bit_depth_luma_minus8: u8,
    pub bit_depth_chroma_minus8: u8,
    pub log2_max_frame_num_minus4: u8,
    pub log2_max_pic_order_cnt_lsb_minus4: u8,
    pub offset_for_non_ref_pic: i8,
    pub offset_for_top_to_bottom_field: i8,
    pub num_ref_frames_in_pic_order_cnt_cycle: u8,
    pub max_num_ref_frames: u8,
    pub pic_width_in_mbs_minus1: u16,
    pub pic_height_in_map_units_minus1: u16,
}

#[repr(C)]
pub struct H264NALPPS { // 4 + 8 bytes = 16 bytes
    pub nt: H264NALType, // from H264NAL

    pub pic_parameter_set_id: u16,
    pub seq_parameter_set_id: u16,
    pub bitfields: u8,
    pub num_slice_groups_minus1: u16,
}
