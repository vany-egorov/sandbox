mod error;

#[macro_use]
extern crate nom;
extern crate clap;
extern crate num_derive;
extern crate num_traits;
extern crate url;

use clap::{App, Arg};
use nom::IResult;
use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::{FromPrimitive, ToPrimitive};
use std::collections::VecDeque;
use std::net::{Ipv4Addr, UdpSocket};
use std::process;
use std::str;
use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::Duration;
use url::{Host /*, ParseError*/, Url};

use error::{Error, Kind as ErrorKind, Result};

const TSSyncByte: u8 = 0x47;
const TSPktSz: usize = 188;
const TSPSIPATSz: usize = 4;

#[derive(Clone, Copy, Debug, FromPrimitive, ToPrimitive, PartialEq)]
pub enum TSPIDKnown {
    PAT = 0x0000,
    CAT = 0x0001,
    TSDT = 0x0002,

    // NIT, ST
    NIT = 0x0010,
    // SDT, BAT, ST
    SDT = 0x0011,
    // EIT, ST CIT (TS 102 323 [13])
    EIT = 0x0012,
    // RST, ST
    RST = 0x0013,
    // TDT, TOT, ST
    TDT = 0x0014,
    // network synchronization
    NetworkSynchronization = 0x0015,
    // RNT (TS 102 323 [13])
    RNT = 0x0016,

    // The PID value 0x001C allocated to link-local
    // inband signalling shall not be used on any broadcast signals.
    // It shall only be used between devices in a controlled environment.
    //
    // NOTE: The PID 0x001C can for example be used within a broadcast centre,
    //       between a receiver device and a
    //       CAM, or on private satellite links
    InbandSignalling = 0x001C,
    Measurement = 0x001D,
    DIT = 0x001E,
    SIT = 0x001F,

    NULL = 0x1FFF,
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum TSPID {
    Known(TSPIDKnown),

    // 0x0003...0x000F
    // 0x0017...0x001B
    Reserved(u16),

    Other(u16),
}

impl TSPID {
    fn from_u16(d: u16) -> Option<TSPID> {
        match TSPIDKnown::from_u16(d) {
            Some(pid) => Some(TSPID::Known(pid)),
            None => match d {
                0x0003..=0x000F | 0x0017..=0x001B => Some(TSPID::Reserved(d)),
                _ => Some(TSPID::Other(d)),
            },
        }
    }

    #[inline]
    fn must_from_u16(d: u16) -> TSPID {
        TSPID::from_u16(d).unwrap_or(TSPID::Other(d))
    }

    fn to_u16(&self) -> Option<u16> {
        match self {
            TSPID::Known(k) => k.to_u16(),
            TSPID::Reserved(d) => Some(*d),
            TSPID::Other(d) => Some(*d),
        }
    }
}

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
    pid: TSPID,

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

// TODO: remove
#[inline]
#[cfg_attr(rustfmt, rustfmt_skip)]
fn parse_take_n(input: &[u8], n: usize) -> IResult<&[u8], &[u8]> {
    do_parse!(input, raw: take!(n) >> (raw))
}

// ETSI EN 300 468 V1.15.1 (2016-03)
// ISO/IEC 13818-1
#[derive(Clone, Debug, FromPrimitive)]
pub enum TSTableIDSingle {
    ProgramAssociationSection = 0x00,
    ConditionalAccessSection = 0x01,
    ProgramMapSection = 0x02,
    TransportStreamDescriptionSection = 0x03,

    NetworkInformationSectionActualNetwork = 0x40,
    NetworkInformationSectionOtherNetwork = 0x41,
    ServiceDescriptionSectionActualTransportStream = 0x42,

    ServiceDescriptionSectionOtherTransportStream = 0x46,
    BouquetAssociationSection = 0x4A,
    EISActualTransportStream = 0x4E,
    EISOtherTransportStream = 0x4F,

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

#[derive(Clone, Debug)]
pub enum TSTableID {
    Single(TSTableIDSingle),

    // 0x50...0x5F
    EISActualTransportStreamSchedule(u8),
    // 0x60...0x6F
    EISOtherTransportStreamSchedule(u8),

    // 0x04...0x3F
    // 0x43...0x45
    Reserved(u8),
}

impl TSTableID {
    fn from_u8(d: u8) -> Option<TSTableID> {
        match TSTableIDSingle::from_u8(d) {
            Some(tidk) => Some(TSTableID::Single(tidk)),
            None => match d {
                0x04..=0x3F | 0x43..=0x45 => Some(TSTableID::Reserved(d)),
                0x50..=0x5F => Some(TSTableID::EISActualTransportStreamSchedule(d)),
                0x60..=0x6F => Some(TSTableID::EISOtherTransportStreamSchedule(d)),
                _ => None,
            },
        }
    }
}

#[derive(Debug)]
pub struct TSPSIHeader {
    // table_id
    // - The table_id identifies to which table the section belongs.
    // - Some table_ids have been defined by ISO and others by ETSI.
    //   Other values of the table_id can be allocated by the user
    //   for private purposes. The list of values of table_id is
    //   contained in table 2.
    // :8
    table_id: Option<TSTableID>,

    // section_syntax_indicator
    // The section_syntax_indicator is a 1-bit
    // field which shall be set to "1".
    // :1
    //
    // reserved_future_use
    // :1
    //
    // reserved_bits
    // :2
    //
    // section-length-unused-bits
    // :2
    //
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
    b1: u8,
    b2: u8,
}

impl TSPSIHeader {
    fn new() -> TSPSIHeader {
        TSPSIHeader {
            table_id: None,
            b1: 0,
            b2: 0,
        }
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSPSIHeader> {
        let mut header = TSPSIHeader::new();

        let (input, raw) = try!(do_parse!(input, raw: take!(3) >> (raw)));

        header.table_id = TSTableID::from_u8(raw[0]);
        header.b1 = raw[1];
        header.b2 = raw[2];

        Ok((input, header))
    }

    #[inline]
    fn section_length(&self) -> u16 {
        ((self.b1 & 0x03) as u16) << 8 | self.b2 as u16
    }
}

// for EIT
#[derive(Debug)]
pub struct TSPSISyntaxSectionDVBEIT {
    // This is a 16-bit field which serves as a label
    // for identification of the TS, about which the EIT
    // informs, from any other multiplex within the delivery system.
    // :16
    transport_stream_id: u16,

    // This 16-bit field gives the label identifying
    // the network_id of the originating delivery system.
    // :16
    original_network_id: u16,

    // This 8-bit field specifies the number of the
    // last section of this segment of the sub_table.
    // For sub_tables which are not segmented, this field shall
    // be set to the same value as the last_section_number field
    // :8
    segment_last_section_number: u8,

    // This 8-bit field identifies the last table_id used
    // :8
    last_table_id: u8,
}

impl TSPSISyntaxSectionDVBEIT {
    fn new() -> TSPSISyntaxSectionDVBEIT {
        TSPSISyntaxSectionDVBEIT {
            transport_stream_id: 0,
            original_network_id: 0,
            segment_last_section_number: 0,
            last_table_id: 0,
        }
    }

    #[inline]
    fn sz() -> usize {
        6
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSPSISyntaxSectionDVBEIT> {
        let mut tss = TSPSISyntaxSectionDVBEIT::new();

        let (input, raw) = try!(do_parse!(
            input,
            raw: take!(TSPSISyntaxSectionDVBEIT::sz()) >> (raw)
        ));

        tss.transport_stream_id = (raw[0] as u16) << 8 | raw[1] as u16;
        tss.original_network_id = (raw[2] as u16) << 8 | raw[3] as u16;
        tss.segment_last_section_number = raw[4];
        tss.last_table_id = raw[5];

        Ok((input, tss))
    }
}

// for SDT
#[derive(Debug)]
pub struct TSPSISyntaxSectionDVBSDT {
    // This 16-bit field gives the label identifying
    // the network_id of the originating delivery system.
    // :16
    original_network_id: u16,

    // :8
    reserved_future_use: u8,
}

impl TSPSISyntaxSectionDVBSDT {
    fn new() -> TSPSISyntaxSectionDVBSDT {
        TSPSISyntaxSectionDVBSDT {
            original_network_id: 0,
            reserved_future_use: 0,
        }
    }

    #[inline]
    fn sz() -> usize {
        3
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSPSISyntaxSectionDVBSDT> {
        let mut tss = TSPSISyntaxSectionDVBSDT::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let (input, raw) = try!(do_parse!(input,
            raw: take!(TSPSISyntaxSectionDVBSDT::sz())
            >> (raw)
        ));

        tss.original_network_id = (raw[0] as u16) << 8 | raw[1] as u16;
        tss.reserved_future_use = raw[2];
        Ok((input, tss))
    }
}

#[derive(Debug)]
pub struct TSPSISyntaxSection {
    // table-id-extension
    // This is a 16-bit field which serves as a label to identify this service from any other service within a TS. The
    // service_id is the same as the program_number in the corresponding program_map_section.
    // :16
    service_id: u16,

    // :2
    // reserved
    //
    // version-number
    // :5
    //
    // curent-next-indicator
    // :1
    b1: u8,

    // section-number
    // :8
    section_number: u8,

    // last-section-number
    // :8
    last_section_number: u8,
}

impl TSPSISyntaxSection {
    fn new() -> TSPSISyntaxSection {
        TSPSISyntaxSection {
            service_id: 0,
            b1: 0,
            section_number: 0,
            last_section_number: 0,
        }
    }

    fn sz() -> usize {
        5
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSPSISyntaxSection> {
        let mut tss = TSPSISyntaxSection::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let (input, raw) = try!(do_parse!(input,
            raw: take!(TSPSISyntaxSection::sz())
            >> (raw)
        ));

        tss.service_id = (raw[0] as u16) << 8 | raw[1] as u16;
        tss.b1 = raw[2];
        tss.section_number = raw[3];
        tss.last_section_number = raw[4];

        Ok((input, tss))
    }

    #[inline]
    fn version_number(&self) -> u8 {
        (self.b1 >> 1) | 0x1F
    }

    #[inline]
    fn current_next_indicator(&self) -> bool {
        (self.b1 | 0x01) == 1
    }
}

// TODO: SDT, EIT are not PSI
// Program Specific Information
//
// From ISO/IEC 13818-1
// The PSI tables are carried in the Transport Stream. There are four PSI tables:
// - PAT - Program Association Table;
// - PMT - Program Map Table;
// - CAT - Conditional Access Table;
// - NIT - Network Information Table.
#[allow(dead_code)]
#[derive(Debug)]
pub struct TSPSI<T>
where
    T: TSPSITableTrait,
{
    header: TSPSIHeader,

    syntax_section: Option<TSPSISyntaxSection>,

    // TODO: join together with enum + table syntax section
    syntax_section_dvb_sdt: Option<TSPSISyntaxSectionDVBSDT>,
    syntax_section_dvb_eit: Option<TSPSISyntaxSectionDVBEIT>,

    // table-data
    data: Option<Vec<T>>,

    // :32
    crc32: u32,
}

impl<'a, T> TSPSI<T>
where
    T: TSPSITableTrait,
{
    fn new() -> TSPSI<T> {
        TSPSI {
            header: TSPSIHeader::new(),

            syntax_section: None,

            syntax_section_dvb_sdt: None,
            syntax_section_dvb_eit: None,

            data: None,

            crc32: 0,
        }
    }

    fn parse_header(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        let (input, header) = try!(TSPSIHeader::parse(input));

        self.header = header;

        Ok((input, ()))
    }

    fn parse_syntax_section(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        let (input, ss) = try!(TSPSISyntaxSection::parse(input));

        self.syntax_section = Some(ss);

        Ok((input, ()))
    }

    fn parse_syntax_section_dvb_sdt(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        let (input, ss_dvb) = try!(TSPSISyntaxSectionDVBSDT::parse(input));

        self.syntax_section_dvb_sdt = Some(ss_dvb);

        Ok((input, ()))
    }

    fn parse_syntax_section_dvb_eit(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        let (input, ss_dvb) = try!(TSPSISyntaxSectionDVBEIT::parse(input));

        self.syntax_section_dvb_eit = Some(ss_dvb);

        Ok((input, ()))
    }

    fn parse_crc32(&mut self, input: &'a [u8]) -> IResult<&'a [u8], ()> {
        #[cfg_attr(rustfmt, rustfmt_skip)]
        let (input, raw) = try!(
            do_parse!(input,
                raw: take!(4)
                >> (raw)
        ));

        self.crc32 =
            (raw[0] as u32) << 24 | (raw[1] as u32) << 16 | (raw[2] as u32) << 8 | (raw[3] as u32);

        Ok((input, ()))
    }

    #[inline]
    fn syntax_section_dvb_sdt_length(&self) -> usize {
        if !self.syntax_section_dvb_sdt.is_none() {
            TSPSISyntaxSectionDVBSDT::sz() // TODO: move to const
        } else {
            0
        }
    }

    #[inline]
    fn syntax_section_dvb_eit_length(&self) -> usize {
        if !self.syntax_section_dvb_eit.is_none() {
            TSPSISyntaxSectionDVBEIT::sz() // TODO: move to const
        } else {
            0
        }
    }

    #[inline]
    fn section_length_data_only(&self) -> usize {
        (self.header.section_length() as usize)
            - 5  // -5 => 5bytes of "PSI - table syntax section";
            - self.syntax_section_dvb_sdt_length()
            - self.syntax_section_dvb_eit_length()
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
        let mut data: Vec<TSPSIPAT> = Vec::new();

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
    fn first_program_map_pid(&self) -> Option<TSPID> {
        self.data
            .as_ref()
            .and_then(|ref data| data.first())
            .and_then(|ref pat| TSPID::from_u16(pat.program_map_pid))
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

        let mut data: Vec<TSPSIPMT> = Vec::new();

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
        let (input, _) = try!(psi.parse_syntax_section_dvb_sdt(input));

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
        let (input, _) = try!(psi.parse_syntax_section_dvb_eit(input));

        // <parse data>
        // limit reader
        let (input, mut raw) = try!(parse_take_n(input, psi.section_length_data_only()));

        let mut data: Vec<TSPSIEIT> = Vec::new();

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

    fn sz() -> usize {
        TSPSIPATSz
    }
}

impl TSPSITableTrait for TSPSIPAT {
    fn kind() -> TSPSITableKind {
        TSPSITableKind::PAT
    }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], TSPSIPAT> {
        let mut pat = TSPSIPAT::new();

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

        pat.program_number = pn;
        pat.program_map_pid = pmp;

        Ok((input, pat))
    }
}

#[derive(Clone, Debug, FromPrimitive, ToPrimitive)]
pub enum TSStreamTypeCommon {
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

    H265 = 0x24,

    ChineseVideoStandard = 0x42,

    IPMPDRM = 0x7F,
    H262DES64CBC = 0x80,
    AC3 = 0x81,          // AC3
    SCTESubtitle = 0x82, // SCTE
    DolbyTrueHDAudio = 0x83,
    AC3DolbyDigitalPlus = 0x84,
    DTS8 = 0x85,
    SCTE35 = 0x86,
    AC3DolbyDigitalPlus16 = 0x87,
}

#[derive(Clone, Debug)]
pub enum TSStreamType {
    // 0x00
    // 0x22
    // 0x23
    // 0x25
    // 0x41
    // 0x43...0x7E
    // 0x88...0x8F
    Reserved(u8),

    Common(TSStreamTypeCommon),
}

impl TSStreamType {
    fn from_u8(d: u8) -> Option<TSStreamType> {
        match TSStreamTypeCommon::from_u8(d) {
            Some(tstc) => Some(TSStreamType::Common(tstc)),
            None => Some(TSStreamType::Reserved(d)),
        }
    }

    fn to_u8(&self) -> Option<u8> {
        match self {
            TSStreamType::Reserved(d) => Some(*d),
            TSStreamType::Common(c) => c.to_u8(),
        }
    }
}

#[derive(Clone, Debug, FromPrimitive, ToPrimitive)]
pub enum TSDescriptorTagCommon {
    VH2621381811172 = 0x02,
    A1381811172 = 0x03,
    Hierarchy = 0x04,
    RegistrationPrivate = 0x05,
    DataStreamAlignment = 0x06,
    TargetBackgroundGrid = 0x07,
    VideoWindow = 0x08,
    CASEMMECMPID = 0x09,
    ISO639 = 0x0A,
    SystemClockExternalReference = 0x0B,
    MultiplexBufferUtilizationBounds = 0x0C,
    CopyrightIdentificationSystemAndReference = 0x0D,
    MaximumBitRate = 0x0E,
    PrivateDataIndicator = 0x0F,
    SmoothingBuffer = 0x10,
    STDVideoBufferLeakControl = 0x11,
    IBPVideoIFrameIndicator = 0x12,
    DSMCCCarouselIdentifier = 0x13,
    DSMCCAssociationTag = 0x14,
    DSMCCDeferredAssociationTag = 0x15,
    DSMCCReserved = 0x16,
    DSMCCNPTReference = 0x17,
    DSMCCNPTEndpoint = 0x18,
    DSMCCStreamMode = 0x19,
    DSMCCStreamEvent = 0x1A,
    VideoStreamHeaderParametersMPEG4H263Based = 0x1B,
    AudioStreamHeaderParametersMPEG4LOASMultiFormatFramed = 0x1C,
    IODParametersForISOIEC144961 = 0x1D,
    SLParametersForISOIEC144961 = 0x1E,
    FMCParametersForISOIEC144961 = 0x1F,
    ExternalESIdentifierForISOIEC144961 = 0x20,
    MuxCodeForISOIEC144961 = 0x21,
    FMXBufferSizeForISOIEC144961 = 0x22,
    MultiplexBufferForISOIEC144961 = 0x23,
    ContentLabelingForISOIEC144961 = 0x24,
    MetadataPointer = 0x25,
    Metadata = 0x26,
    MetadataSTD = 0x27,
    VideoStreamHeaderParametersForITUTRecH264AndISOIEC1449610 = 0x28,
    ISOIEC1381811IPMPDRM = 0x29,
    TimingAndHRDForITUTRecH264andISOIEC1449610 = 0x2A,
    AudioStreamHeaderParametersForISOIEC138187ADTSAAC = 0x2B,
    FlexMuxTimingForISOIEC144961 = 0x2C,
    TextStreamHeaderParametersForISOIEC14496 = 0x2D,
    AudioExtensionStreamHeaderParametersForISOIEC144963MPEG4LOASMultiFormatFramed = 0x2E,
    VideoAuxiliaryStreamHeaderParameters = 0x2F,
    VideoScalableStreamHeaderParameters = 0x30,
    VideoMultiStreamHeaderParameters = 0x31,
    VideoStreamHeaderParametersForITUTRecT800AndISOIEC15444 = 0x32,
    VideoMultiOperationPointStreamHeaderParameters = 0x33,
    VideoStereoscopic3DStreamHeaderParametersForITUTRecH262ISOIEC138182AndISOIEC111722 = 0x34,
    ProgramStereoscopic3DInformation = 0x35,
    VideoStereoscopic3DInformation = 0x36,

    VideoLANFourCCVideoSizeAndCodecInitializationData = 0xA0,
}

#[derive(Clone, Debug, FromPrimitive, ToPrimitive)]
pub enum TSDescriptorTagDVB {
    NetworkName = 0x40,
    ServiceList = 0x41,
    Stuffing = 0x42,
    SatelliteDeliverySystem = 0x43,
    CableDeliverySystem = 0x44,
    VBIData = 0x45,
    VBITeletext = 0x46,
    BouquetName = 0x47,
    Service = 0x48,
    CountryAvailability = 0x49,
    Linkage = 0x4A,
    NVODReference = 0x4B,
    TimeShiftedService = 0x4C,
    ShortEvent = 0x4D,
    ExtendedEvent = 0x4E,
    TimeShiftedEvent = 0x4F,
    Component = 0x50,
    Mosaic = 0x51,
    StreamIdentifier = 0x52,
    CAIdentifier = 0x53,
    Content = 0x54,
    ParentalRating = 0x55,
    Teletext = 0x56,
    Telephone = 0x57,
    LocalTimeOffset = 0x58,
    Subtitling = 0x59,
    TerrestrialDeliverySystem = 0x5A,
    MultilingualNetworkName = 0x5B,
    MultilingualBouquetName = 0x5C,
    MultilingualServiceName = 0x5D,
    MultilingualComponent = 0x5E,
    PrivateDataSpecifier = 0x5F,
    ServiceMove = 0x60,
    ShortSmoothingBuffer = 0x61,
    FrequencyList = 0x62,
    PartialTransportStream = 0x63,
    DataBroadcast = 0x64,
    Scrambling = 0x65,
    DataBroadcastId = 0x66,
    TransportStream = 0x67,
    DSNG = 0x68,
    PDC = 0x69,
    AC3 = 0x6A,
    AncillaryData = 0x6B,
    CellList = 0x6C,
    CellFrequencyLink = 0x6D,
    AnnouncementSupport = 0x6E,
    ApplicationSignalling = 0x6F,
    AdaptationFieldData = 0x70,
    ServiceIdentifier = 0x71,
    ServiceAvailability = 0x72,
    DefaultAuthority = 0x73,
    RelatedContent = 0x74,
    TVAId = 0x75,
    ContentIdentifier = 0x76,
    TimeSliceFecIdentifier = 0x77,
    ECMRepetitionRate = 0x78,
    S2SatelliteDeliverySystem = 0x79,
    EnhancedAC3 = 0x7A,
    DTSDescriptor = 0x7B,
    AAC = 0x7C,
    XAITLocation = 0x7D,
    FTAContentManagement = 0x7E,
    Extension = 0x7F,
}

#[derive(Clone, Debug)]
pub enum TSDescriptorTag {
    // 0x00...0x01
    // 0x37...0x3F
    Reserved(u8),

    // 0x02..0x36
    Common(TSDescriptorTagCommon),

    // 0x40...0x7F
    DVB(TSDescriptorTagDVB),

    // 0x80...0x9F | 0xA1...0xCF
    ATSC(u8),

    // 0xD0...0xDF
    ISDB(u8),

    // 0xE0...0xE9
    CableLabs(u8),

    // 0xEA...0xFE
    Other(u8),

    // 0xFF
    Forbidden(u8),
}

impl TSDescriptorTag {
    fn from_u8(d: u8) -> Option<TSDescriptorTag> {
        match d {
            0x00..=0x01 => Some(TSDescriptorTag::Reserved(d)),

            0x02..=0x36 | 0xA0 => {
                TSDescriptorTagCommon::from_u8(d).and_then(|tag| Some(TSDescriptorTag::Common(tag)))
            }

            0x37..=0x3F => Some(TSDescriptorTag::Reserved(d)),

            0x40..=0x7F => {
                TSDescriptorTagDVB::from_u8(d).and_then(|tag| Some(TSDescriptorTag::DVB(tag)))
            }

            0x80..=0x9F | 0xA1..=0xCF => Some(TSDescriptorTag::ATSC(d)),

            0xD0..=0xDF => Some(TSDescriptorTag::ISDB(d)),

            0xE0..=0xE9 => Some(TSDescriptorTag::CableLabs(d)),

            0xEA..=0xFE => Some(TSDescriptorTag::Other(d)),

            0xFF => Some(TSDescriptorTag::Forbidden(d)),
        }
    }

    fn to_u8(&self) -> Option<u8> {
        match self {
            TSDescriptorTag::Reserved(d) => Some(*d),
            TSDescriptorTag::Common(c) => c.to_u8(),
            TSDescriptorTag::DVB(dvb) => dvb.to_u8(),
            TSDescriptorTag::ATSC(d) => Some(*d),
            TSDescriptorTag::ISDB(d) => Some(*d),
            TSDescriptorTag::CableLabs(d) => Some(*d),
            TSDescriptorTag::Other(d) => Some(*d),
            TSDescriptorTag::Forbidden(d) => Some(*d),
        }
    }
}

#[derive(Clone, Debug)]
pub struct TSDescriptor {
    // the tag defines the structure of the
    // contained data following the descriptor length.
    // :8
    tag: Option<TSDescriptorTag>,

    // the number of bytes that are to follow.
    // :8
    len: u8,

    data: Option<Vec<u8>>,
}

impl TSDescriptor {
    fn new() -> TSDescriptor {
        TSDescriptor {
            tag: None,
            len: 0,

            data: None,
        }
    }

    fn data_as_str(&self) -> &str {
        self.data
            .as_ref()
            .and_then(|ref data| str::from_utf8(data).ok())
            .unwrap_or("---")
    }

    fn tag_to_u8(&self) -> u8 {
        self.tag
            .as_ref()
            .unwrap_or(&TSDescriptorTag::Reserved(0))
            .to_u8()
            .unwrap_or(0)
    }

    fn parse(input: &[u8]) -> IResult<&[u8], TSDescriptor> {
        let mut d = TSDescriptor::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(input, (tag, len, data)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(take_bits!(u8, 8))
            >> bn: take!(b2)

            >> (
                b1, // tag
                b2, // len
                bn // data
            )
        ));

        d.tag = TSDescriptorTag::from_u8(tag);
        d.len = len;
        d.data = Some(data.to_vec());

        println!(
            "[t] [dsc]     (:tag ({:?}/0x{:02X}/{}) :len {} :data {})",
            d.tag,
            d.tag_to_u8(),
            d.tag_to_u8(),
            d.len,
            d.data_as_str()
        );

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
        let(input, (st, pid, dsc_len)) = try!(do_parse!(input,
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

        println!(
            "[t] [pmt]   (:type ({:?}) :pid {} :len {})",
            s.stream_type, pid, dsc_len
        );

        // limit-reader
        let (input, mut raw) = try!(parse_take_n(input, dsc_len as usize));

        if dsc_len > 0 {
            let mut descriptors: Vec<TSDescriptor> = Vec::new();

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
    // pcr_pid
    // The packet identifier that contains the program clock reference used to
    // improve the random access accuracy of the stream's timing that is
    // derived from the program timestamp. If this is unused.
    // then it is set to 0x1FFF (all bits on).
    // :13
    b1: u8,
    b2: u8,

    // reserved bits (set to 0x0F (all bits on))
    // :4
    //
    // program info length unused bits (set to 0 (all bits off))
    // :2
    //
    // program_info_length
    // The number of bytes that follow for the program descriptors.
    // :10
    b3: u8,
    b4: u8,

    // program descriptors
    descriptors: Option<Vec<TSDescriptor>>,

    // elementary stream info data
    streams: Option<Vec<TSPSIPMTStream>>,
}

impl TSPSIPMT {
    fn new() -> TSPSIPMT {
        TSPSIPMT {
            b1: 0,
            b2: 0,

            b3: 0,
            b4: 0,

            descriptors: None,

            streams: None,
        }
    }

    #[inline]
    fn sz() -> u8 {
        4
    }

    #[inline]
    fn pcr_pid(&self) -> u16 {
        ((self.b1 & 0x1F) as u16) << 8 | self.b2 as u16
    }

    // #[inline]
    // fn set_pcr_pid(&mut self, v: u16) {}

    #[inline]
    fn program_info_length(&self) -> u16 {
        ((self.b3 & 0x03) as u16) << 8 | self.b4 as u16
    }
}

impl TSPSITableTrait for TSPSIPMT {
    fn kind() -> TSPSITableKind {
        TSPSITableKind::PMT
    }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], Self> {
        let mut pmt = TSPSIPMT::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(mut input, head) = try!(do_parse!(input,
                raw: take!(TSPSIPMT::sz() as usize)
                >> (raw))
        );

        pmt.b1 = head[0];
        pmt.b2 = head[1];

        pmt.b3 = head[2];
        pmt.b3 = head[3];

        {
            let len = pmt.program_info_length();
            if len > 0 {
                input = try!(parse_take_n(input, len as usize)).0;
            }
        }

        let mut streams: Vec<TSPSIPMTStream> = Vec::new();

        while input.len() > 0 {
            let (tail, stream) = try!(TSPSIPMTStream::parse(input));
            streams.push(stream);

            input = tail;
        }

        pmt.streams = Some(streams);

        Ok((input, pmt))
    }
}

// Service Description Table
//
// the SDT contains data describing the services
// in the system e.g. names of services, the service provider, etc.
#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct TSPSISDT {
    // This is a 16-bit field which serves as a label
    // to identify this service from any other service within the TS.
    // The service_id is the same as the program_number
    // in the corresponding program_map_section.
    // :16
    service_id: u16,

    // reserved future use
    // :6
    //
    // This is a 1-bit field which when set to "1"
    // indicates that EIT schedule information for the service is
    // present in the current TS, see ETR 211 [7] for
    // information on maximum time interval between
    // occurrences of an EIT schedule sub_table).
    // If the flag is set to 0 then the EIT schedule information
    // for the service should not be present in the TS.
    // :1
    //
    // This is a 1-bit field which when set to "1" indicates
    // that EIT_present_following information for the service
    // is present in the current TS, see ETR 211 [7] for
    // information on maximum time interval between occurrences
    // of an EIT present/following sub_table).
    // If the flag is set to 0 then the EIT present/following
    // information for the service should not be present in the TS.
    // :1
    b1: u8,

    // running status
    // :3
    //
    // free CA mode
    // This 1-bit field, when set to "0"
    // indicates that all the component streams
    // of the service are not scrambled.
    // When set to "1" it indicates that access to
    // one or more streams may be controlled by a CA system.
    // :1
    //
    // descriptors-loop-len.0
    // :4
    b2: u8,

    // descriptors-loop-len.1
    // :8
    b3: u8,

    descriptors: Option<Vec<TSDescriptor>>,
}

impl TSPSISDT {
    fn new() -> TSPSISDT {
        TSPSISDT {
            service_id: 0,
            b1: 0,
            b2: 0,
            b3: 0,
            descriptors: None,
        }
    }

    #[inline]
    fn descriptors_length(&self) -> u16 {
        ((self.b2 & 0x0F) as u16) << 8 | self.b3 as u16
    }
}

impl TSPSITableTrait for TSPSISDT {
    fn kind() -> TSPSITableKind {
        TSPSITableKind::SDT
    }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], TSPSISDT> {
        let mut sdt = TSPSISDT::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(input, raw) = try!(
            do_parse!(input,
                raw: take!(5)
                >> (raw))
        );

        sdt.service_id = ((raw[0] as u16) << 8) | raw[1] as u16;
        sdt.b1 = raw[2];
        sdt.b2 = raw[3];
        sdt.b3 = raw[4];

        // limit-reader
        let dsc_len = sdt.descriptors_length();
        let (input, mut raw) = try!(do_parse!(input, raw: take!(dsc_len) >> (raw)));

        if dsc_len > 0 {
            let mut descriptors: Vec<TSDescriptor> = Vec::new();

            while raw.len() > 0 {
                let (tail, descriptor) = try!(TSDescriptor::parse(raw));
                descriptors.push(descriptor);

                raw = tail;
            }

            sdt.descriptors = Some(descriptors);
        }

        Ok((input, sdt))
    }
}

// Event Information Table
// - the EIT contains data concerning events
//   or programmes such as event name, start time, duration, etc.;
// - the use of different descriptors allows the transmission of different kinds of event information e.g. for
//   different service types.
#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct TSPSIEIT {
    // This 16-bit field contains the identification number
    // of the described event (uniquely allocated within a
    // service definition).
    // :16
    event_id: u16,

    // This 40-bit field contains the start time of the event in Universal Time,
    // Co-ordinated (UTC) and Modified Julian Date (MJD) (see annex C).
    // This field is coded as 16 bits giving the 16 LSBs
    // of MJD followed by 24 bits coded as
    // 6 digits in 4-bit Binary Coded Decimal (BCD).
    // If the start time is undefined (e.g. for an event in a NVOD reference
    // service) all bits of the field are set to "1".
    //
    // EXAMPLE 1: 93/10/13 12:45:00 is coded as "0xC079124500".
    //
    // :40
    start_time_b: [u8; 5],

    // A 24-bit field containing the duration of the event in hours, minutes, seconds. format: 6 digits,
    // 4-bit BCD = 24 bit.
    //
    // EXAMPLE 2: 01:45:30 is coded as "0x014530".
    //
    // :24
    duration_b: [u8; 3],

    // running_status
    // This is a 3-bit field indicating the status of the event as defined in table 6.
    // For an NVOD reference event the value of the running_status
    // shall be set to "0".
    // :3
    //
    // free_CA_mode
    // This 1-bit field, when set to "0" indicates that all
    // the component streams of the event are not scrambled.
    // When set to "1" it indicates that access to one or
    // more streams is controlled by a CA system.
    // :1
    //
    // descriptors_loop_length.0
    // This 12-bit field gives the total
    // length in bytes of the following descriptors.
    // :4
    //
    // :8
    b1: u8,

    // descriptors_loop_length.1
    // :8
    b2: u8,

    descriptors: Option<Vec<TSDescriptor>>,
}

impl TSPSIEIT {
    fn new() -> TSPSIEIT {
        TSPSIEIT {
            event_id: 0,
            start_time_b: [0; 5],
            duration_b: [0; 3],
            b1: 0,
            b2: 0,
            descriptors: None,
        }
    }

    #[inline]
    fn descriptors_length(&self) -> u16 {
        ((self.b1 & 0x0F) as u16) << 8 | self.b2 as u16
    }
}

impl TSPSITableTrait for TSPSIEIT {
    fn kind() -> TSPSITableKind {
        TSPSITableKind::EIT
    }

    fn parse<'a>(input: &'a [u8]) -> IResult<&'a [u8], TSPSIEIT> {
        let mut eit = TSPSIEIT::new();

        #[cfg_attr(rustfmt, rustfmt_skip)]
        let(input, (event_id, st_b, db_b, b1, b2)) = try!(do_parse!(input,
               b1: bits!(take_bits!(u8, 8))
            >> b2: bits!(take_bits!(u8, 8))

            >> b3: take!(5)
            >> b4: take!(3)

            >> b5: bits!(take_bits!(u8, 8))
            >> b6: bits!(take_bits!(u8, 8))

            >> (
                ((b1 as u16) << 8) | b2 as u16,
                b3, b4, b5, b6
            )
        ));

        eit.event_id = event_id;
        eit.start_time_b.clone_from_slice(st_b);
        eit.duration_b.clone_from_slice(db_b);
        eit.b1 = b1;
        eit.b2 = b2;

        // limit-reader
        let dsc_len = eit.descriptors_length();
        let (input, mut raw) = try!(parse_take_n(input, dsc_len as usize));

        if dsc_len > 0 {
            let mut descriptors: Vec<TSDescriptor> = Vec::new();

            while raw.len() > 0 {
                let (tail, descriptor) = try!(TSDescriptor::parse(raw));
                descriptors.push(descriptor);

                raw = tail;
            }

            eit.descriptors = Some(descriptors);
        }

        Ok((input, eit))
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
        let(input, (sid, p_len)) = try!(do_parse!(input,
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

        println!(
            "[t] [PES] (:stream-id {} :packet-length {})",
            p.stream_id, p.packet_length
        );

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
            pid: TSPID::must_from_u16(
                ((b1.3 as u16) << 8) | b2 as u16),
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
    tag!(&[TSSyncByte])      >> // 1
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
    buf: Arc<(Mutex<VecDeque<[u8; TSPktSz]>>, Condvar)>,

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
            let mut ts_pkt_raw: [u8; TSPktSz] = [0; TSPktSz];

            loop {
                // MTU (maximum transmission unit) == 1500 for Ethertnet
                // 7*TSPktSz = 7*188 = 1316 < 1500 => OK
                let mut pkts_raw = [0; 7 * TSPktSz];
                let (_, _) = socket.recv_from(&mut pkts_raw).unwrap();

                let &(ref lock, ref cvar) = &*pair;
                let mut buf = lock.lock().unwrap();

                for pkt_index in 0..7 * TSPktSz / TSPktSz {
                    let ts_pkt_raw_src = &pkts_raw[pkt_index * TSPktSz..(pkt_index + 1) * TSPktSz];

                    // println!("#{:?} -> [{:?} .. {:?}]; src-len: {:?}, dst-len: {:?}",
                    //     pkt_index, pkt_index*TSPktSz, (pkt_index+1)*TSPktSz,
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
                let (tail, _) = try!(parse_ts_adaptation(&ts_pkt_raw));
                ts_pkt_raw = tail;
            }

            if !ts_header.contains_payload {
                continue;
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
                // uint8_t* section_start = payload + *payload + 1;
                // ts_pkt_raw = &ts_pkt_raw[((ts_pkt_raw[0] as usize) + 1)..];
                ts_pkt_raw = try!(do_parse!(
                    ts_pkt_raw,
                    raw: take!(ts_pkt_raw[0] + 1) >> (raw)
                ))
                .0;
            }

            match ts_header.pid {
                TSPID::Known(TSPIDKnown::PAT) if self.ts.pat.is_none() => {
                    let (_, psi) = try!(TSPSI::parse_pat(ts_pkt_raw));
                    self.ts.pat = Some(psi);

                    println!("[+] (:PAT (:pid {:?} :cc {}))", ts_header.pid, ts_header.cc);
                }

                TSPID::Known(TSPIDKnown::SDT) if self.ts.sdt.is_none() => {
                    let (_, psi) = try!(TSPSI::parse_sdt(ts_pkt_raw));
                    println!("[+] (:SDT {:?})", psi);
                    self.ts.sdt = Some(psi);
                }

                TSPID::Known(TSPIDKnown::EIT) => {
                    let (_, psi) = try!(TSPSI::parse_eit(ts_pkt_raw));
                    println!("[+] (:EIT {:?})", psi);
                }

                _ => {}
            }

            if self.ts.pmt.is_none() && !self.ts.pat.is_none() {
                let pid_pmt = self
                    .ts
                    .pat
                    .as_ref()
                    .and_then(|pat| pat.first_program_map_pid());

                if pid_pmt == Some(ts_header.pid) {
                    let (_, psi) = try!(TSPSI::parse_pmt(ts_pkt_raw));
                    self.ts.pmt = Some(psi);

                    println!(
                        "[+] (:PMT (:pid {:?}/0x{:02X} :cc {}))",
                        ts_header.pid,
                        ts_header.pid.to_u16().unwrap_or(0),
                        ts_header.cc
                    );
                }
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
        println!("<<< File open {}", self.url);

        Ok(())
    }

    fn read(&mut self) -> Result<()> {
        thread::sleep(Duration::from_secs(1000));

        Ok(())
    }

    fn close(&mut self) -> Result<()> {
        println!("<<< File close {}", self.url);

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
