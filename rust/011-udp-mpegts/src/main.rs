mod error;

#[macro_use]
extern crate nom;
extern crate clap;
extern crate url;
extern crate num_derive;
extern crate num_traits;

use clap::{App, Arg};
use nom::IResult;
use std::collections::VecDeque;
use std::net::{Ipv4Addr, UdpSocket};
use std::process;
use std::str;
use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::Duration;
use url::{Host /*, ParseError*/, Url};
use num_derive::FromPrimitive;
use num_traits::FromPrimitive;

use error::{Error, Kind as ErrorKind, Result};

const TS_SYNC_BYTE: u8 = 0x47;
const TS_PKT_SZ: usize = 188;

const TS_PID_PAT: u16 = 0x0000;
const TS_PID_CAT: u16 = 0x0001;
const TS_PID_NIT: u16 = 0x0010;
const TS_PID_SDT: u16 = 0x0011;
const TS_PID_EIT: u16 = 0x0012;
const TS_PID_NULL: u16 = 0x1FFF;

const TS_PSI_PAT_SZ: usize = 4;

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSHeader {
    // transcport-error-indicator
    // :1
    tei: u8,

    // payload-unit-start-indicator
    // Set when a PES, PSI, or DVB-MIP
    // packet begins immediately following the header.
    // :1
    pusi: bool,

    // transport-priority
    // :1
    tp: u8,

    // :13
    pid: u16,

    // transport-scrambling-control
    // :2
    tsc: u8,

    // adaptation-field-control
    // :1
    afc: u8,

    // :1
    contains_payload: bool,

    // continuity-counter
    // :4
    cc: u8,
}

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSPCR {
    // :33
    base: u64,

    // :9
    ext: u16,
}

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSAdaptation {
    // adaptation-field-length
    // :8
    afl: u8,

    // discontinuity-indicator
    // :1
    di: u8,

    // random-access-indicator
    // :1
    rai: u8,

    // elementary-stream-priority-indicator
    // :1
    espi: u8,

    // PCR-flag
    // :1
    pcr_flag: u8,

    // OPCR-flag
    // :1
    opcr_flag: u8,

    // splicing-point-flag
    // :1
    spf: u8,

    // transport-private-data-flag
    // :1
    tpdf: u8,

    // adaptation-field-extension-flag
    // :1
    afef: u8,

    pcr: Option<TSPCR>,
}

#[inline]
#[cfg_attr(rustfmt, rustfmt_skip)]
fn parse_take_n(input: &[u8], n: usize) -> IResult<&[u8], &[u8]> {
    do_parse!(input,
       raw: take!(n)
    >> (raw))
}


// ETSI EN 300 468 V1.3.1 (1998-02)
// ETSI EN 300 468 V1.11.1 (2010-04)
#[derive(Clone, Debug, FromPrimitive)]
pub enum TSTableID {
    ProgramAssociationSection = 0x00,
    ConditionalAccessSection = 0x01,
    ProgramMapSection = 0x02,
    TransportStreamDescriptionSection = 0x03,

    NetworkInformationSectionActualNetwork = 0x40,
    NetworkInformationSectionOtherNetwork = 0x41,
    ServiceDescriptionSectionActualTransportStream = 0x42,
    ServiceDescriptionSectionOtherTransportStream = 0x46,
    BouquetAssociationSection = 0x4A,
    EventInformationSectionActualTransportStream = 0x4E,
    EventInformationSectionOtherTransportStream = 0x4F,

    TimeDateSection = 0x70,
    RunningStatusSection = 0x71,
    StuffingSection = 0x72,
    TimeOffsetSection = 0x73,
    ApplicationInformationSection = 0x74,
    ContainerSection = 0x75,
    RelatedContentSection = 0x76,
    ContentIdentifierSection = 0x77,
    MPEFECSection = 0x78,
    ResolutionNotificationSection = 0x79,
    MPEIFECSection = 0x7A,
    DiscontinuityInformationSection = 0x7E,
    SelectionInformationSection = 0x7F,
}

// Program Specific Information
#[allow(dead_code)]
#[derive(Debug)]
pub struct TSPSI<T>
where
    T: TSPSITableTrait,
{
    // PSI - header
    table_id: Option<TSTableID>,

    // section-syntax-indicator
    // :1
    ssi: u8,

    // :1
    private_bit: u8,

    // :2
    reserved_bits: u8,

    // section-length-unused-bits
    // :2
    slub: u8,

    // table syntax section length
    //
    // This is a 12-bit field, the first two bits of which shall be "00".
    // It specifies the number of bytes of the
    // section, starting immediately following
    // the section_length field and including the CRC.
    // The section_length shall not
    // exceed 1 021 so that the entire section has a maximum length
    // of 1 024 bytes.
    //
    // :10
    section_length: u16,

    // <PSI - table syntax section>
    // :16
    tsi: u16,

    // version-number
    // :5
    vn: u8,

    // curent-next-indicator
    // :1
    cni: u8,

    // section-number
    // :8
    sn: u8,

    // last-section-number
    // :8
    lsn: u8,

    // table-data
    data: Option<Vec<T>>,

    // :32
    crc32: u32,
    // </PSI - table syntax section>
}

impl<'a, T> TSPSI<T>
where
    T: TSPSITableTrait,
{
    fn new() -> TSPSI<T> {
        TSPSI {
            table_id: None,
            ssi: 0,
            private_bit: 0,
            reserved_bits: 0,
            slub: 0,
            section_length: 0,

            tsi: 0,
            vn: 0,
            cni: 0,
            sn: 0,
            lsn: 0,

            data: None,

            crc32: 0,
        }
    }

    // PSI - header - 3bytes
    fn parse_header(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(input, (tid, ssi, pb, rb, slub, slen)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(tuple!(
                take_bits!(u8, 1),
                take_bits!(u8, 1),
                take_bits!(u8, 2),
                take_bits!(u8, 2),
                take_bits!(u8, 2)
            ))
            >> b3: bits!(take_bits!(u8, 8))
            >> (
                b1,  // table-id
                b2.0,  // ssi
                b2.1,  // private-bit
                b2.2,  // reserved-bits
                b2.3,  // slub
                ((b2.4 as u16) << 8) | b3 as u16  // section-length
            )
        ));

        self.table_id = TSTableID::from_u8(tid);
        self.ssi = ssi;
        self.private_bit = pb;
        self.reserved_bits = rb;
        self.slub = slub;
        self.section_length = slen;

        Ok((input, ()))
    }

    // PSI - table syntax section - 5bytes
    fn parse_syntax_section(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        #[cfg_attr(rustfmt, rustfmt_skip)]
        let (input, (tsi, vn, cni, sn, lsn)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(take_bits!(u8, 8))
            >> b3: bits!(tuple!(
                take_bits!(u8, 2),
                take_bits!(u8, 5),
                take_bits!(u8, 1)
            ))
            >> b4: bits!(take_bits!(u8, 8))
            >> b5: bits!(take_bits!(u8, 8))

            >> (
                ((b1 as u16) << 8) | b2 as u16,  // tsi
                b3.1,  // vn
                b3.2,  // cni
                b4,  // sn
                b5  // lsn
            )
        ));

        self.tsi = tsi;
        self.vn = vn;
        self.cni = cni;
        self.sn = sn;
        self.lsn = lsn;

        Ok((input, ()))
    }

    fn parse_crc32(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        #[cfg_attr(rustfmt, rustfmt_skip)]
        let (input, v) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(take_bits!(u8, 8))
            >> b3: bits!(take_bits!(u8, 8))
            >> b4: bits!(take_bits!(u8, 8))

            >> (
                (b1 as u32) << 24 |
                (b2 as u32) << 16 |
                (b3 as u32) << 8  |
                (b4 as u32)
            )
        ));

        self.crc32 = v;

        Ok((input, ()))
    }

    #[inline]
    fn section_length_data_only(&self) -> usize {
        (self.section_length as usize)
            - 5  // -5 => 5bytes of "PSI - table syntax section";
            - 4 // -4 => 4bytes of CRC32;
    }

    fn check_crc32() -> bool {
        // TODO: crc32 check here
        true
    }
}

impl TSPSI<TSPSIPAT> {
    fn parse_pat(input: &[u8]) -> IResult<&[u8], TSPSI<TSPSIPAT>> {
        let mut psi = TSPSI::new();

        let (input, _) = try!(psi.parse_header(input));
        let (input, _) = try!(psi.parse_syntax_section(input));

        // <parse data>

        // not working for nom 4.x.y
        // see:
        // https://github.com/Geal/nom/issues/790
        //
        // let (_, data) = try!(do_parse!(raw,
        //     d: many1!(parse_ts_psi_pat_datum) >>
        //     (d)
        // ));

        // limit reader
        let (input, mut raw) = try!(parse_take_n(input, psi.section_length_data_only()));

        let sz = TSPSIPAT::sz();
        let mut data: Vec<TSPSIPAT> = vec![TSPSIPAT::new(); raw.len() % sz];

        while raw.len() >= sz {
            let (tail, pat) = try!(TSPSIPAT::parse(&raw));

            data.push(pat);
            raw = tail;
        }

        psi.data = Some(data);
        // </parse data>

        let (input, _) = try!(psi.parse_crc32(input));

        Ok((input, psi))
    }

    #[inline]
    fn first_program_map_pid(&self) -> Option<u16> {
        self.data
            .as_ref()
            .and_then(|ref data| { data.first() })
            .and_then(|ref pat| { Some(pat.program_map_pid) })
    }
}

impl TSPSI<TSPSIPMT> {
    fn parse_pmt(input: &[u8]) -> IResult<&[u8], TSPSI<TSPSIPMT>> {
        let mut psi = TSPSI::new();

        let (input, _) = try!(psi.parse_header(input));
        let (input, _) = try!(psi.parse_syntax_section(input));

        // <parse data>
        // limit reader
        let (input, mut raw) = try!(parse_take_n(input, psi.section_length_data_only()));

        let mut data: Vec<TSPSIPMT> = vec![TSPSIPMT::new(); 1];

        while raw.len() > 0 {
            let (tail, pmt) = try!(TSPSIPMT::parse(&raw));

            data.push(pmt);
            raw = tail;
        }

        psi.data = Some(data);
        // </parse data>

        let (input, _) = try!(psi.parse_crc32(input));

        Ok((input, psi))
    }
}

impl TSPSI<TSPSISDT> {
    fn parse_sdt(input: &[u8]) -> IResult<&[u8], TSPSI<TSPSISDT>> {
        let mut psi = TSPSI::new();

        let (input, _) = try!(psi.parse_header(input));
        let (input, _) = try!(psi.parse_syntax_section(input));

        // <parse data>
        // limit reader
        let (input, mut raw) = try!(parse_take_n(input, psi.section_length_data_only()));

        let mut data: Vec<TSPSISDT> = vec![TSPSISDT::new(); 1];

        while raw.len() > 0 {
            let (tail, sdt) = try!(TSPSISDT::parse(&raw));

            data.push(sdt);
            raw = tail;
        }

        psi.data = Some(data);
        // </parse data>

        let (input, _) = try!(psi.parse_crc32(input));

        Ok((input, psi))
    }
}

impl TSPSI<TSPSIEIT> {
    fn parse_eit(input: &[u8]) -> IResult<&[u8], TSPSI<TSPSIEIT>> {
        let mut psi = TSPSI::new();

        let (input, _) = try!(psi.parse_header(input));
        let (input, _) = try!(psi.parse_syntax_section(input));

        // <parse data>
        // limit reader
        let (input, mut raw) = try!(parse_take_n(input, psi.section_length_data_only()));

        let mut data: Vec<TSPSIEIT> = vec![TSPSIEIT::new(); 1];

        while raw.len() > 0 {
            let (tail, eit) = try!(TSPSIEIT::parse(&raw));

            data.push(eit);
            raw = tail;
        }

        psi.data = Some(data);
        // </parse data>

        let (input, _) = try!(psi.parse_crc32(input));

        Ok((input, psi))
    }
}

pub trait TSPSITableTrait: Sized {
    fn kind() -> TSPSITableKind;
    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], Self>;
}

#[derive(Clone, Debug)]
pub enum TSPSITableKind {
    PAT, // Program Association Table
    PMT, // Program Map Table
    SDT, // Service Description Table
    EIT, // Event Information Table

    CAT, // Conditional Access
    NIT, // Network Information Table
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct TSPSIPAT {
    // Relates to the Table ID extension in the associated PMT.
    // A value of 0 is reserved for a NIT packet identifier.
    program_number: u16,

    // The packet identifier that
    // contains the associated PMT
    program_map_pid: u16,
}

impl TSPSIPAT {
    fn new() -> TSPSIPAT {
        TSPSIPAT {
            program_number: 0,
            program_map_pid: 0,
        }
    }

    fn sz() -> usize { TS_PSI_PAT_SZ }
}

impl TSPSITableTrait for TSPSIPAT {
    fn kind() -> TSPSITableKind { TSPSITableKind::PAT }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], TSPSIPAT> {
        let mut psi = TSPSIPAT::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(input, (pn, pmp)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(take_bits!(u8, 8))

            >> b3: bits!(tuple!(
                take_bits!(u8, 3),
                take_bits!(u8, 5)
            ))
            >> b4: bits!(take_bits!(u8, 8))

            >> (
                ((b1 as u16) << 8) | b2 as u16, // program_number
                ((b3.1 as u16) << 8) | b4 as u16 // program_map_pid
            )
        ));

        psi.program_number = pn;
        psi.program_map_pid = pmp;

        Ok((input, psi))
    }
}

#[derive(Clone, Debug, FromPrimitive)]
pub enum TSStreamType {
    Reserved0x00 = 0x00,

    MPEG1Video = 0x01,
    H262 = 0x02,
    MPEG1Audio = 0x03, // mp2
    MPEG2Audio = 0x04, // mp2
    MPEG2TabledData = 0x05,
    MPEG2PacketizedData = 0x06, // mp3
    MHEG = 0x07,
    DSMCCInAPacketizedStream = 0x08,
    H222AuxiliaryData = 0x09,
    DSMCCMultiprotocolEncapsulation = 0x0A,
    DSMCCUNMessages = 0x0B,
    DSMCCStreamDescriptors = 0x0C,
    DSMCCTabledData = 0x0D,
    ISOIEC138181AuxiliaryData = 0x0E,
    AAC = 0x0F, // AAC
    MPEG4H263Video = 0x10,
    MPEG4LOAS = 0x11,
    MPEG4FlexMux = 0x12,
    MPEG4FlexMuxTables = 0x13,
    DSMCCSynchronizedDownloadProtocol = 0x14,
    PacketizedMetadata = 0x15,
    SectionedMetadata = 0x16,
    DSMCCDataCarouselMetadata = 0x17,
    DSMCCObjectCarouselMetadata = 0x18,
    SynchronizedDownloadProtocolMetadata = 0x19,
    IPMP = 0x1A,
    H264 = 0x1B,
    MPEG4RawAudio = 0x1C,
    MPEG4Text = 0x1D,
    MPEG4AuxiliaryVideo = 0x1E,
    SVC = 0x1F,
    MVC = 0x20,
    JPEG2000Video = 0x21,

    Reserved0x22 = 0x22,
    Reserved0x23 = 0x23,

    H265 = 0x24,

    Reserved0x25 = 0x25,
    // ...
    Reserved0x41 = 0x41,

    ChineseVideoStandard = 0x42,

    Reserved0x43 = 0x43,
    // ...
    Reserved0x7E = 0x7E,

    IPMPDRM = 0x7F,
    H262DES64CBC = 0x80,
    AC3 = 0x81, // AC3
    SCTESubtitle = 0x82, // SCTE
    DolbyTrueHDAudio = 0x83,
    AC3DolbyDigitalPlus = 0x84,
    DTS8 = 0x85,
    SCTE35 = 0x86,
    AC3DolbyDigitalPlus16 = 0x87,

    Reserved0x88 = 0x88,
    // ...
    Reserved0x8F = 0x8F,
}

#[derive(Clone, Debug)]
pub struct TSDescriptor {
    // the tag defines the structure of the
    // contained data following the descriptor length.
    // :8
    tag: u8,

    // the number of bytes that are to follow.
    // :8
    len: u8,

    data: Option<Vec<u8>>,
}

impl TSDescriptor {
    fn new() -> TSDescriptor {
        TSDescriptor {
            tag: 0,
            len: 0,

            data: None,
        }
    }

    fn data_as_str(&self) -> &str {
        self.data
            .as_ref()
            .and_then(|ref data| { str::from_utf8(data).ok() })
            .unwrap_or("---")
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSDescriptor> {
        let mut d = TSDescriptor::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(mut input, (tag, len, data)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(take_bits!(u8, 8))
            >> bn: take!(b2)

            >> (
                b1,  // tag
                b2,  // len
                bn  // data
            )
        ));

        d.tag = tag;
        d.len = len;
        d.data = Some(data.to_vec());

        println!("[t] [pmt]     (:tag ({:?}) :len {} :data {})",
            d.tag, d.len, d.data_as_str());

        Ok((input, d))
    }
}

#[derive(Clone, Debug)]
pub struct TSPSIPMTStream {
    // This defines the structure of the data
    // contained within the elementary packet identifier.
    // :8
    stream_type: Option<TSStreamType>,

    // reserved bits (set to 0x07 (all bits on))
    // :3
    //
    // The packet identifier that contains the stream type data.
    // :13
    pid: u16,

    // reserved bits (set to 0x0F (all bits on))
    // :4
    //
    // ES Info length unused bits (set to 0 (all bits off))
    // :2
    //
    // The number of bytes that follow for the elementary stream descriptors.
    // :10
    descriptors_length: u16,

    // When the ES info length is non-zero,
    // this is the ES info length number of elementary stream descriptor bytes.
    descriptors: Option<Vec<TSDescriptor>>,
}

impl TSPSIPMTStream {
    fn new() -> TSPSIPMTStream {
        TSPSIPMTStream {
            stream_type: None,
            pid: 0,

            descriptors_length: 0,
            descriptors: None,
        }
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSPSIPMTStream> {
        let mut s = TSPSIPMTStream::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(mut input, (st, pid, dsc_len)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(tuple!(
                take_bits!(u8, 3),
                take_bits!(u8, 5)
            ))
            >> b3: bits!(take_bits!(u8, 8))
            >> b4: bits!(tuple!(
                take_bits!(u8, 4),
                take_bits!(u8, 2),
                take_bits!(u8, 2)
            ))
            >> b5: bits!(take_bits!(u8, 8))

            >> (
                b1,  // stream_type
                ((b2.1 as u16) << 8) | b3 as u16,  // pid
                ((b4.2 as u16) << 8) | b5 as u16  // descriptors_length
            )
        ));

        s.stream_type = TSStreamType::from_u8(st);
        s.pid = pid;
        s.descriptors_length = dsc_len;

        println!("[t] [pmt]   (:type ({:?}) :pid {} :len {})",
            s.stream_type, pid, dsc_len);

        // limit-reader
        let(input, mut raw) = try!(parse_take_n(input, dsc_len as usize));

        if dsc_len > 0 {
            let mut descriptors = vec![TSDescriptor::new(); 2];

            while raw.len() > 0 {
                let (tail, descriptor) = try!(TSDescriptor::parse(raw));
                descriptors.push(descriptor);

                raw = tail;
            }

            s.descriptors = Some(descriptors);
        }

        Ok((input, s))
    }
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct TSPSIPMT {
    // TODO: handle 0x1FFF
    //
    // reserved_bits
    // :3
    //
    // The packet identifier that contains the program clock reference used to
    // improve the random access accuracy of the stream's timing that is
    // derived from the program timestamp. If this is unused.
    // then it is set to 0x1FFF (all bits on).
    // :13
    pcr_pid: u16,

    // reserved bits (set to 0x0F (all bits on))
    // :4
    //
    // program info length unused bits (set to 0 (all bits off))
    // :2
    //
    // program_info_length
    // The number of bytes that follow for the program descriptors.
    // :10
    pi_length: u16,

    // program descriptors
    descriptors: Option<Vec<TSDescriptor>>,

    // elementary stream info data
    streams: Option<Vec<TSPSIPMTStream>>,
}

impl TSPSIPMT {
    fn new() -> TSPSIPMT {
        TSPSIPMT {
            pcr_pid: 0,
            pi_length: 0,

            descriptors: None,

            streams: None,
        }
    }
}

impl TSPSITableTrait for TSPSIPMT {
    fn kind() -> TSPSITableKind { TSPSITableKind::PMT }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], Self> {
        let mut psi = TSPSIPMT::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(mut input, (pcr_pid, pi_length)) = try!(do_parse!(input,
            b1: bits!(tuple!(
                take_bits!(u8, 3),
                take_bits!(u8, 5)
            ))
            >> b2: bits!(take_bits!(u8, 8))
            >> b3: bits!(tuple!(
                take_bits!(u8, 4),
                take_bits!(u8, 2),
                take_bits!(u8, 2)
            ))
            >> b4: bits!(take_bits!(u8, 8))

            >> (
                ((b1.1 as u16) << 8) | b2 as u16, // pcr_pid
                ((b3.2 as u16) << 8) | b4 as u16 // program_info_length
            )
        ));

        psi.pcr_pid = pcr_pid;
        psi.pi_length = pi_length;

        if pi_length > 0 {
            input = try!(parse_take_n(input, pi_length as usize)).0;
        }

        println!("[t] [pmt] :pcr-pid {}", pcr_pid);

        let mut streams = vec![TSPSIPMTStream::new(); 5];

        while input.len() > 0 {
            let (tail, stream) = try!(TSPSIPMTStream::parse(input));
            streams.push(stream);

            input = tail;
        }

        psi.streams = Some(streams);

        Ok((input, psi))
    }
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct TSPSISDT {}

impl TSPSISDT {
    fn new() -> TSPSISDT {
        TSPSISDT {}
    }
}

impl TSPSITableTrait for TSPSISDT {
    fn kind() -> TSPSITableKind {
        TSPSITableKind::SDT
    }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], TSPSISDT> {
        let psi = TSPSISDT::new();

        Ok((input, psi))
    }
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct TSPSIEIT {}

impl TSPSIEIT {
    fn new() -> TSPSIEIT {
        TSPSIEIT {}
    }
}

impl TSPSITableTrait for TSPSIEIT {
    fn kind() -> TSPSITableKind {
        TSPSITableKind::EIT
    }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], TSPSIEIT> {
        let psi = TSPSIEIT::new();

        Ok((input, psi))
    }
}

pub struct TSPESOptionalHeader {
    // marker_bits              :2
    // scrambling_control       :2
    // priority                 :1
    // data_alignment_indicator :1
    // copyright                :1
    // original_or_copy         :1
    b1: u8,

    // PTS_DTS_indicator         :2
    // ESCR_flag                 :1
    // ES_rate_flag              :1
    // DSM_trick_mode_flag       :1
    // additional_copy_info_flag :1
    // CRC_flag                  :1
    // extension_flag            :1
    b2: u8,

    header_length: u8,

    dts: Option<u64>,
    pts: Option<u64>,
}

pub struct TSPES {
    stream_id: u8,

    packet_length: u16,

    header: Option<TSPESOptionalHeader>,
}

impl TSPES {
    fn new() -> TSPES {
        TSPES {
            stream_id: 0,
            packet_length: 0,
            header: None,
        }
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSPES> {
        let mut p = TSPES::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(mut input, (sid, p_len)) = try!(do_parse!(input,
            _start_code: tag!(&[0x00, 0x00, 0x01])  // TODO: move to PESStartCode as const

            >> b1: bits!(take_bits!(u8, 8))

            >> b2: bits!(take_bits!(u8, 8))
            >> b3: bits!(take_bits!(u8, 8))

            >> (
                b1,
                ((b2 as u16) << 8) | b3 as u16
            )
        ));

        p.stream_id = sid;
        p.packet_length = p_len;

        println!("[t] [PES] (:stream-id {} :packet-length {})",
            p.stream_id, p.packet_length);

        Ok((input, p))
    }
}

pub struct TS {
    pat: Option<TSPSI<TSPSIPAT>>,
    pmt: Option<TSPSI<TSPSIPMT>>,
    sdt: Option<TSPSI<TSPSISDT>>,
}

impl TS {
    fn new() -> TS {
        TS {
            pat: None,
            pmt: None,
            sdt: None,
        }
    }
}

#[cfg_attr(rustfmt, rustfmt_skip)]
pub fn parse_ts_header(input: &[u8]) -> IResult<&[u8], TSHeader> {
    do_parse!(input,
        b1: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 5)
        ))
        >> b2: bits!(take_bits!(u8, 8))
        >> b3: bits!(tuple!(
            take_bits!(u8, 2),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 4)
        ))

        >> (TSHeader {
            tei: b1.0,
            pusi: b1.1 != 0,
            tp: b1.2,
            pid: ((b1.3 as u16) << 8) | b2 as u16,
            tsc: b3.0,
            afc: b3.1,
            contains_payload: b3.2 != 0,
            cc: b3.3,
        })
    )
}

#[cfg_attr(rustfmt, rustfmt_skip)]
pub fn parse_ts_adaptation(input: &[u8]) -> IResult<&[u8], TSAdaptation> {;
    let (input, mut ts_adaptation) = try!(do_parse!(input,
           b1: bits!(take_bits!(u8, 8))
        >> b2: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1)
        ))

        >> (TSAdaptation {
            afl: b1,

            di: b2.0,
            rai: b2.1,
            espi: b2.2,
            pcr_flag: b2.3,
            opcr_flag: b2.4,
            spf: b2.5,
            tpdf: b2.6,
            afef: b2.7,

            pcr: None,
        })
    ));

    if ts_adaptation.pcr_flag == 0 {
        return Ok((input, ts_adaptation))
    }

    let (input, ts_pcr) = try!(do_parse!(input,
           b1: bits!(take_bits!(u8, 8))
        >> b2: bits!(take_bits!(u8, 8))
        >> b3: bits!(take_bits!(u8, 8))
        >> b4: bits!(take_bits!(u8, 8))
        >> b5: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 6),
            take_bits!(u8, 1)
        ))
        >> b6: bits!(take_bits!(u8, 8))

        >> (TSPCR {
            base: (
                ((b1 as u64) << 25) |
                ((b2 as u64) << 17) |
                ((b3 as u64) << 9) |
                ((b4 as u64) << 1) |
                 (b5.0 as u64)
            ),
            ext: (
                ((b5.2 as u16) << 8) |
                 (b6 as u16)
            ),
        })
    ));

    ts_adaptation.pcr = Some(ts_pcr);

    Ok((input, ts_adaptation))
}

#[cfg_attr(rustfmt, rustfmt_skip)]
named!(parse_ts_single<&[u8], TSHeader>, do_parse!(
    tag!(&[TS_SYNC_BYTE])      >> // 1
    ts_header: parse_ts_header >> // 3

    (ts_header)));

struct DemuxerTS {}

impl DemuxerTS {
    pub fn new() -> DemuxerTS {
        DemuxerTS {}
    }
}

trait Input {
    fn open(&mut self) -> Result<()>;
    fn read(&mut self) -> Result<()>;
    fn close(&mut self) -> Result<()>;
}

trait Filter {
    fn consume_strm(&self);
    fn consume_trk(&self);
    fn consume_pkt_raw(&self);
    fn consume_pkt(&self);
    fn consume_frm(&self);

    fn produce_strm(&self);
    fn produce_trk(&self);
    fn produce_pkt_raw(&self);
    fn produce_pkt(&self);
    fn produce_frm(&self);
}

struct InputUDP {
    url: Url,

    // circullar-buffer / fifo
    buf: Arc<(Mutex<VecDeque<[u8; TS_PKT_SZ]>>, Condvar)>,

    ts: TS,

    socket: Option<UdpSocket>,
}

impl InputUDP {
    pub fn new(url: Url, buf_cap: usize) -> InputUDP {
        InputUDP {
            url: url,
            buf: Arc::new((Mutex::new(VecDeque::with_capacity(buf_cap)), Condvar::new())),

            ts: TS::new(),

            socket: None,
        }
    }
}

impl Input for InputUDP {
    fn open(&mut self) -> Result<()> {
        let input_host = try!(self
            .url
            .host()
            .ok_or(Error::new(ErrorKind::InputUrlMissingHost, "")));

        let input_port = self.url.port().unwrap_or(5500);

        let input_host_domain = try!(match input_host {
            Host::Domain(v) => Ok(v),
            _ => Err(Error::new(ErrorKind::InputUrlHostMustBeDomain, "")),
        });

        let iface = Ipv4Addr::new(0, 0, 0, 0);
        // let socket = try!(UdpSocket::bind((input_host_domain, input_port)));;

        // let iface = Ipv4Addr::new(127, 0, 0, 1);
        println!(
            "[<] {:?}: {:?} @ {:?}",
            input_host_domain, input_port, iface
        );

        let input_host_ip_v4: Ipv4Addr = input_host_domain.parse().unwrap();

        let socket = try!(UdpSocket::bind((input_host_domain, input_port)));

        if let Err(e) = socket.join_multicast_v4(&input_host_ip_v4, &iface) {
            eprintln!("error join-multiocast-v4: {}", e);
        }

        let pair = self.buf.clone();
        thread::spawn(move || {
            let mut ts_pkt_raw: [u8; TS_PKT_SZ] = [0; TS_PKT_SZ];

            loop {
                // MTU (maximum transmission unit) == 1500 for Ethertnet
                // 7*TS_PKT_SZ = 7*188 = 1316 < 1500 => OK
                let mut pkts_raw = [0; 7 * TS_PKT_SZ];
                let (_, _) = socket.recv_from(&mut pkts_raw).unwrap();

                let &(ref lock, ref cvar) = &*pair;
                let mut buf = lock.lock().unwrap();

                for pkt_index in 0..7 * TS_PKT_SZ / TS_PKT_SZ {
                    let ts_pkt_raw_src =
                        &pkts_raw[pkt_index * TS_PKT_SZ..(pkt_index + 1) * TS_PKT_SZ];

                    // println!("#{:?} -> [{:?} .. {:?}]; src-len: {:?}, dst-len: {:?}",
                    //     pkt_index, pkt_index*TS_PKT_SZ, (pkt_index+1)*TS_PKT_SZ,
                    //     ts_pkt_raw_src.len(), ts_pkt_raw.len(),
                    // );

                    ts_pkt_raw.copy_from_slice(ts_pkt_raw_src);
                    buf.push_back(ts_pkt_raw);
                }

                cvar.notify_all();
            }
        });

        Ok(())
    }

    fn read(&mut self) -> Result<()> {
        let pair = self.buf.clone();
        let &(ref lock, ref cvar) = &*pair;
        let mut buf = try!(lock
            .lock()
            .ok()
            .ok_or(Error::new(ErrorKind::SyncPoison, "udp read lock error")));

        buf = try!(cvar.wait(buf).ok().ok_or(Error::new(
            ErrorKind::SyncPoison,
            "udp read cwar wait erorr"
        )));

        while !buf.is_empty() {
            // TODO: move to function;
            let ts_pkt_raw = buf.pop_front().unwrap();

            // parse header
            let (mut ts_pkt_raw, ts_header) = try!(parse_ts_single(&ts_pkt_raw));

            if ts_header.afc == 1 {
                let (tail, ts_adaptation) = try!(parse_ts_adaptation(&ts_pkt_raw));
                ts_pkt_raw = tail;

                // println!(
                //     "adaptation (:pcr? {:?} :adaptation-field-length {:?})",
                //     ts_adaptation.pcr_flag, ts_adaptation.afl
                // );

                if let Some(ref pcr) = ts_adaptation.pcr {
                    // println!(
                    //     "pcr: {} / {} / 0:0:0:XXX ({})",
                    //     pcr.base,
                    //     pcr.ext,
                    //     pcr.base * 300,
                    // );
                }
            }

            if !ts_header.contains_payload {
                continue
            }

            if ts_header.pusi {
                // payload data start
                //
                // https://stackoverflow.com/a/27525217
                // From the en300 468 spec:
                //
                // Sections may start at the beginning of the payload of a TS packet,
                // but this is not a requirement, because the start of the first
                // section in the payload of a TS packet is pointed to by the pointer_field.
                //
                // So the section start actually is an offset from the payload:
                //
                // uint8_t* section_start = payload + *payload + 1
                ts_pkt_raw = &ts_pkt_raw[((ts_pkt_raw[0] as usize) + 1)..];
            }

            if ts_header.pid == TS_PID_PAT && self.ts.pat.is_none() {
                let (_, psi) = try!(TSPSI::parse_pat(ts_pkt_raw));
                self.ts.pat = Some(psi);

                println!(
                    "[+] (:PAT (:pid 0x{:04X}/{} :cc {}))",
                    ts_header.pid, ts_header.pid, ts_header.cc
                );

            } else if let Some(ref pat) = self.ts.pat {
                if self.ts.pmt.is_none() && Some(ts_header.pid) == pat.first_program_map_pid() {
                    let (_, psi) = try!(TSPSI::parse_pmt(ts_pkt_raw));
                    self.ts.pmt = Some(psi);

                    println!(
                        "[+] (:PMT (:pid 0x{:04X}/{} :cc {}))",
                        ts_header.pid, ts_header.pid, ts_header.cc
                    );
                }

            } else if ts_header.pid == TS_PID_SDT && self.ts.sdt.is_none() {
                let (_, psi) = try!(TSPSI::parse_sdt(ts_pkt_raw));
                self.ts.sdt = Some(psi);

            } else if ts_header.pid == TS_PID_EIT {
                let (_, psi) = try!(TSPSI::parse_eit(ts_pkt_raw));
                println!("[+] (:EIT {:?})", psi);

            }
        }

        Ok(())
    }

    fn close(&mut self) -> Result<()> {
        println!("<<< UDP close");

        Ok(())
    }
}

struct InputFile {
    url: Url,
}

impl InputFile {
    pub fn new(url: Url) -> InputFile {
        InputFile { url: url }
    }
}

impl Input for InputFile {
    fn open(&mut self) -> Result<()> {
        println!("<<< File open");

        Ok(())
    }

    fn read(&mut self) -> Result<()> {
        thread::sleep(Duration::from_secs(1000));

        Ok(())
    }

    fn close(&mut self) -> Result<()> {
        println!("<<< File close");

        Ok(())
    }
}

struct Wrkr<I> {
    input: Arc<Mutex<I>>,
}

impl<I> Wrkr<I>
where
    I: Input + std::marker::Send + 'static,
{
    pub fn new(input: I) -> Wrkr<I> {
        Wrkr {
            input: Arc::new(Mutex::new(input)),
        }
    }

    pub fn run(&self) -> Result<()> {
        let input = self.input.clone();
        {
            try!(input.lock().unwrap().open());
        }

        thread::spawn(move || loop {
            input.lock().unwrap().read();
        });

        Ok(())
    }
}

// Input #0, mpegts, from 'udp://239.255.1.1:5500':
//   Duration: N/A, start: 8174.927322, bitrate: N/A
//   Program 20105
//     Metadata:
//       service_name    : ?HD
//       service_provider: ~~~
//     Stream #0:0[0xcd]: Video: h264 (High) ([27][0][0][0] / 0x001B), yuv420p(tv, bt709), 1920x1080 [SAR 1:1 DAR 16:9], 25 fps, 50 tbr, 90k tbn, 50 tbc
//     Stream #0:1[0x131](rus): Audio: mp2 ([4][0][0][0] / 0x0004), 48000 Hz, stereo, s16p, 192 kb/s
//     Stream #0:2[0x195](rus): Audio: ac3 (AC-3 / 0x332D4341), 48000 Hz, 5.1(side), fltp, 384 kb/s
//     Stream #0:3[0x1f9](rus,rus): Subtitle: dvb_teletext ([6][0][0][0] / 0x0006)
fn main() {
    // let args: Vec<String> = env::args().collect();
    // println!("{:?}", args);
    let matches = App::new("V/A tool")
        .version("0.0.1")
        .author("Ivan Egorov <vany.egorov@gmail.com>")
        .about("Video/audio swiss knife")
        .arg(
            Arg::with_name("input")
                // .index(1)
                .short("i")
                .long("input")
                .help("Sets the input file to use")
                .required(true)
                .takes_value(true),
        )
        .get_matches();

    let input_raw = matches.value_of("input").unwrap();
    let input_url = match Url::parse(input_raw) {
        Ok(v) => v,
        Err(err) => {
            eprintln!("error parse input url: {:?}\n", err);
            process::exit(1);
        }
    };

    let input_url_1 = input_url.clone();
    let input_url_2 = input_url.clone();

    // <input builder based on URL>
    let input_udp = InputUDP::new(input_url_1, 5000 * 7);
    let input_file = InputFile::new(input_url_2);
    // </input builder based on URL>

    let wrkr1 = Wrkr::new(input_udp);
    let wrkr2 = Wrkr::new(input_file);

    if let Err(err) = wrkr1.run() {
        eprintln!("error start worker №1: {:?}\n", err);
        process::exit(1);
    }

    if let Err(err) = wrkr2.run() {
        eprintln!("error start worker №2: {:?}\n", err);
        process::exit(1);
    }

    loop {
        thread::sleep(Duration::from_secs(60));
    }
}
