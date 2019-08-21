use std::fmt;
use std::time::Duration;

use duration_fmt::DurationFmt;
use error::{Error, Kind as ErrorKind};
use pid::PID;
use rational;
use rational::Rational;
use result::Result;
use table_id::TableID;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum TransportScramblingControl {
    NotScrambled,
    ScrambledReserved,
    ScrambledEven,
    ScrambledOdd,
}

impl From<u8> for TransportScramblingControl {
    #[inline(always)]
    fn from(d: u8) -> Self {
        match d {
            0b00 => TransportScramblingControl::NotScrambled,
            0b01 => TransportScramblingControl::ScrambledReserved,
            0b10 => TransportScramblingControl::ScrambledEven,
            0b11 => TransportScramblingControl::ScrambledOdd,

            _ => TransportScramblingControl::NotScrambled,
        }
    }
}

/// Program clock reference,
/// stored as 33 bits base, 6 bits reserved, 9 bits extension.
/// The value is calculated as base * 300 + extension.
pub struct PCR<'buf> {
    buf: &'buf [u8],
}

impl<'buf> PCR<'buf> {
    const SZ: usize = 6;
    const TB: Rational = rational::TB_27MHZ;

    #[inline(always)]
    fn new(buf: &'buf [u8]) -> PCR<'buf> {
        PCR { buf }
    }

    #[inline(always)]
    #[allow(dead_code)]
    fn try_new(buf: &'buf [u8]) -> Result<PCR<'buf>> {
        let a = Self::new(buf);
        a.validate()?;
        Ok(a)
    }

    #[inline(always)]
    fn validate(&self) -> Result<()> {
        if self.buf.len() < Self::SZ {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::SZ)))
        } else {
            Ok(())
        }
    }

    #[inline(always)]
    fn base(&self) -> u64 {
        ((self.buf[0] as u64) << 25)
            | ((self.buf[1] as u64) << 17)
            | ((self.buf[2] as u64) << 9)
            | ((self.buf[3] as u64) << 1)
            | (((self.buf[4] & 0b1000_0000) >> 7) as u64)
    }

    #[inline(always)]
    fn ext(&self) -> u16 {
        (((self.buf[4] & 0b0000_00001) as u16) << 8) | (self.buf[5] as u16)
    }

    /// 27MHz
    pub fn value(&self) -> u64 {
        self.base() * 300 + (self.ext() as u64)
    }

    /// nanoseconds
    pub fn ns(&self) -> u64 {
        rational::rescale(self.value(), Self::TB, rational::TB_1NS)
    }
}

impl<'buf> From<&PCR<'buf>> for Duration {
    fn from(pcr: &PCR) -> Self {
        Duration::from_nanos(pcr.ns())
    }
}

impl<'buf> From<&PCR<'buf>> for DurationFmt {
    fn from(pcr: &PCR) -> Self {
        DurationFmt::from_nanos(pcr.ns())
    }
}

impl<'buf> fmt::Debug for PCR<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:PCR (:raw {:08X}:{:04X} :v(27MHz) {} :duration {}))",
            self.base(),
            self.ext(),
            self.value(),
            DurationFmt::from(self)
        )
    }
}

trait WithBuf<'buf> {
    #[inline(always)]
    fn buf(&self) -> &'buf [u8];
}

const TABLE_HEADER_SZ: usize = 3;
#[allow(dead_code)]
const TABLE_HEADER_MAX_SECTION_LENGTH: usize = 0x3FD; // 1021

trait WithTableHeader<'buf>: WithBuf<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        self.buf()
    }

    /// table_id
    /// - The table_id identifies to which table the section belongs.
    /// - Some table_ids have been defined by ISO and others by ETSI.
    ///   Other values of the table_id can be allocated by the user
    ///   for private purposes.
    #[inline(always)]
    fn table_id(&self) -> TableID {
        TableID::from(self.b()[0])
    }

    /// This is a 12-bit field, the first two bits of which shall be '00'.
    /// The remaining 10 bits specify the number of bytes of the section,
    /// starting immediately following the section_length field,
    /// and including the CRC. The value in this
    /// field shall not exceed 1021 (0x3FD)
    #[inline(always)]
    fn section_length(&self) -> u16 {
        (((self.b()[1] & 0b0000_1111) as u16) << 8) | self.b()[2] as u16
    }
}

const TABLE_SYNTAX_SECTION_SZ: usize = 5;

trait WithTableSyntaxSection<'buf>: WithBuf<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        &self.buf()[TABLE_HEADER_SZ..]
    }

    /// This is a 16-bit field which serves as a label to identify
    /// this Transport Stream from any other multiplex within a network.
    /// Its value is defined by the user.
    #[inline(always)]
    #[allow(dead_code)]
    fn transport_stream_id(&self) -> u16 {
        ((self.b()[0] as u16) << 8) | (self.b()[1] as u16)
    }

    /// This 5-bit field is the version number of the whole
    /// Program Association Table. The version number
    /// shall be incremented by 1 modulo 32 whenever the definition
    /// of the Program Association Table changes. When the
    /// current_next_indicator is set to '1', then the version_number
    /// shall be that of the currently applicable Program Association
    /// Table. When the current_next_indicator is set to '0',
    /// then the version_number shall be that of the next applicable Program
    /// Association Table.
    #[inline(always)]
    #[allow(dead_code)]
    fn version_number(&self) -> u8 {
        (self.b()[2] & 0b0011_1110) >> 1
    }

    /// A 1-bit indicator, which when set to '1' indicates
    /// that the Program Association Table sent is currently applicable.
    /// When the bit is set to '0', it indicates that the table
    // sent is not yet applicable and shall be the next table to become valid.
    #[inline(always)]
    fn current_next_indicator(&self) -> bool {
        (self.b()[2] & 0b0000_0001) != 0
    }

    /// This 8-bit field gives the number of this section.
    /// The section_number of the first section in the Program Association
    /// Table shall be 0x00. It shall be incremented by 1
    /// with each additional section in the Program Association Table.
    #[inline(always)]
    fn section_number(&self) -> u8 {
        self.b()[3]
    }

    /// This 8-bit field specifies the number of the last section
    /// (that is, the section with the highest section_number)
    /// of the complete Program Association Table.
    #[inline(always)]
    fn last_section_number(&self) -> u8 {
        self.b()[4]
    }
}

const TABLE_CRC32_SZ: usize = 4;

trait WithTableCRC32<'buf>: WithBuf<'buf> {
}

/// ISO/IEC 13818-1
///
/// Program association Table
/// Associates Program Number and Program Map Table PID
///
/// The Program Association Table provides the correspondence
/// between a program_number and the PID value of the
/// Transport Stream packets which carry the program definition.
/// The program_number is the numeric label associated with
/// a program.
pub struct TablePAT<'buf> {
    buf: &'buf [u8],
}

impl<'buf> TablePAT<'buf> {
    #[allow(dead_code)]
    const SZ: usize = TABLE_HEADER_SZ + TABLE_SYNTAX_SECTION_SZ;

    #[inline(always)]
    fn new(buf: &'buf [u8]) -> TablePAT<'buf> {
        TablePAT { buf }
    }

    // #[inline(always)]
    // fn program_number(&self) -> u16 {
    //     ((self.buf[0] as u16) << 8) | self.buf[1] as u16
    // }

    // #[inline(always)]
    // fn program_map_pid(&self) -> u16 {
    //     (((self.buf[2] & 0b0001_1111) as u16) << 8) | self.buf[3] as u16
    // }
}

impl<'buf> WithBuf<'buf> for TablePAT<'buf> {
    fn buf(&self) -> &'buf [u8] {
        self.buf
    }
}

impl<'buf> WithTableHeader<'buf> for TablePAT<'buf> {}
impl<'buf> WithTableSyntaxSection<'buf> for TablePAT<'buf> {}
impl<'buf> WithTableCRC32<'buf> for TablePAT<'buf> {}

impl<'buf> fmt::Debug for TablePAT<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:PAT (:program-number {} :program-map-pid {}))",
            // self.program_number(),
            1,
            // self.program_map_pid()
            1,
        )
    }
}

pub struct TablePMT<'buf> {
    buf: &'buf [u8],
}

pub struct Adaptation<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Adaptation<'buf> {
    const HEADER_SZ: usize = 1;
    const HEADER_FULL_SZ: usize = 2;

    #[inline(always)]
    fn new(buf: &'buf [u8]) -> Adaptation<'buf> {
        Adaptation { buf }
    }

    #[inline(always)]
    fn try_new(buf: &'buf [u8]) -> Result<Adaptation<'buf>> {
        let a = Self::new(buf);
        a.validate()?;
        Ok(a)
    }

    #[inline(always)]
    fn validate(&self) -> Result<()> {
        if self.buf.len() < Self::HEADER_SZ {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::HEADER_SZ)))
        } else if self.buf.len() < self.sz() {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), self.sz())))
        } else {
            Ok(())
        }
    }

    #[inline(always)]
    fn sz(&self) -> usize {
        Self::HEADER_SZ + self.field_length()
    }

    /// number of bytes in the adaptation field
    /// immediately following this byte
    #[inline(always)]
    fn field_length(&self) -> usize {
        self.buf[0] as usize
    }

    /// set if current TS packet is in a discontinuity
    /// state with respect to either the continuity
    /// counter or the program clock reference
    #[inline(always)]
    #[allow(dead_code)]
    fn discontinuity_indicator(&self) -> bool {
        (self.buf[1] & 0b1000_0000) != 0
    }

    /// set when the stream may be decoded without
    /// errors from this point
    #[inline(always)]
    #[allow(dead_code)]
    fn random_access_indicator(&self) -> bool {
        (self.buf[1] & 0b0100_0000) != 0
    }

    /// set when this stream should be considered "high priority"
    #[inline(always)]
    pub fn elementary_stream_priority_indicator(&self) -> bool {
        (self.buf[1] & 0b0010_0000) != 0
    }

    /// set when PCR field is present
    #[inline(always)]
    fn pcr_flag(&self) -> bool {
        (self.buf[1] & 0b0001_0000) != 0
    }

    /// set when OPCR field is present
    #[inline(always)]
    pub fn opcr_flag(&self) -> bool {
        (self.buf[1] & 0b0000_1000) != 0
    }

    /// set when splice countdown field is present
    #[inline(always)]
    pub fn splicing_point_flag(&self) -> bool {
        (self.buf[1] & 0b0000_0100) != 0
    }

    /// set when transport private data is present
    #[inline(always)]
    pub fn transport_private_data_flag(&self) -> bool {
        (self.buf[1] & 0b0000_0010) != 0
    }

    /// set when transport private data is present
    #[inline(always)]
    pub fn adaptation_field_extension_flag(&self) -> bool {
        (self.buf[1] & 0b0000_0001) != 0
    }

    /// seek to PCR start position
    #[inline(always)]
    fn buf_seek_pcr(&self) -> &'buf [u8] {
        &self.buf[Self::HEADER_FULL_SZ..]
    }

    /// seek to OPCR start position
    #[inline(always)]
    fn buf_seek_opcr(&self) -> &'buf [u8] {
        let mut buf = self.buf_seek_pcr();
        if self.pcr_flag() {
            buf = &buf[PCR::SZ..];
        }
        buf
    }

    #[inline(always)]
    pub fn pcr(&self) -> Option<PCR<'buf>> {
        if self.pcr_flag() {
            Some(PCR::new(self.buf_seek_pcr()))
        } else {
            None
        }
    }

    /// Original Program clock reference.
    /// Helps when one TS is copied into another.
    #[inline(always)]
    pub fn opcr(&self) -> Option<PCR> {
        if self.opcr_flag() {
            Some(PCR::new(self.buf_seek_opcr()))
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn splice_countdown(&self) -> Option<u8> {
        if self.splicing_point_flag() {
            // TODO: implement
            unimplemented!()
        } else {
            None
        }
    }
}

pub struct Header<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Header<'buf> {
    const SZ: usize = 4;

    #[inline(always)]
    fn new(buf: &'buf [u8]) -> Header<'buf> {
        Header { buf }
    }

    /// Set when a demodulator can't correct errors from FEC data;
    /// indicating the packet is corrupt.
    #[inline(always)]
    #[allow(dead_code)]
    fn tei(&self) -> bool {
        (self.buf[1] & 0b1000_0000) != 0
    }

    /// Set when a PES, PSI, or DVB-MIP
    /// packet begins immediately following the header.
    #[inline(always)]
    fn pusi(&self) -> bool {
        (self.buf[1] & 0b0100_0000) != 0
    }

    /// Set when the current packet has a higher
    /// priority than other packets with the same PID.
    #[inline(always)]
    #[allow(dead_code)]
    fn transport_priority(&self) -> bool {
        (self.buf[1] & 0b0010_0000) != 0
    }

    /// Packet Identifier, describing the payload data.
    #[inline(always)]
    fn pid(&self) -> PID {
        PID::from((((self.buf[1] & 0b0001_1111) as u16) << 8) | self.buf[2] as u16)
    }

    /// transport-scrambling-control
    #[inline(always)]
    #[allow(dead_code)]
    fn tsc(&self) -> TransportScramblingControl {
        TransportScramblingControl::from((self.buf[3] & 0b1100_0000) >> 6)
    }

    #[inline(always)]
    fn got_adaptation(&self) -> bool {
        (self.buf[3] & 0b0010_0000) != 0
    }

    #[inline(always)]
    fn got_payload(&self) -> bool {
        (self.buf[3] & 0b0001_0000) != 0
    }

    #[inline(always)]
    pub fn cc(&self) -> u8 {
        self.buf[3] & 0b0000_1111
    }
}

pub struct Packet<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Packet<'buf> {
    const SZ: usize = 188;
    const SYNC_BYTE: u8 = 0x47;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Result<Packet<'buf>> {
        let pkt = Packet { buf };

        pkt.validate()?;

        Ok(pkt)
    }

    #[inline(always)]
    fn validate(&self) -> Result<()> {
        if self.buf.len() != Self::SZ {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::SZ)))
        } else if self.buf[0] != Self::SYNC_BYTE {
            Err(Error::new(ErrorKind::SyncByte(self.buf[0])))
        } else {
            Ok(())
        }
    }

    /// adaptation start position
    #[inline(always)]
    fn buf_pos_adaptation() -> usize {
        Header::SZ
    }

    // TODO: try_seek?
    //       or pos_<name> + seek?
    /// position payload start
    #[inline(always)]
    fn buf_pos_payload(&self) -> usize {
        let mut pos = Self::buf_pos_adaptation();
        let header = self.header();

        if header.got_adaptation() {
            // TODO: Adaptation::sz(self.buf)
            //       self.adaptation() + self.try_adaptation()
            let adapt = Adaptation::new(self.buf_seek(pos));
            pos += adapt.sz();
        }

        if header.pusi() {
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
            pos += (self.buf[pos] as usize) + 1;
        }

        pos
    }

    #[inline(always)]
    fn buf_seek(&self, offset: usize) -> &'buf [u8] {
        &self.buf[offset..]
    }

    #[inline(always)]
    fn buf_try_seek(&self, offset: usize) -> Result<&'buf [u8]> {
        if self.buf.len() <= offset {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::SZ)))
        } else {
            Ok(self.buf_seek(offset))
        }
    }

    // TODO: merge Header and Packet?
    #[inline(always)]
    fn header(&self) -> Header<'buf> {
        Header::new(self.buf)
    }

    #[inline(always)]
    fn adaptation(&self) -> Option<Result<Adaptation<'buf>>> {
        let header = self.header();

        if header.got_adaptation() {
            // TODO: move to macro? or optional-result crate
            match self.buf_try_seek(Self::buf_pos_adaptation()) {
                Ok(buf) => Some(Adaptation::try_new(buf)),
                Err(e) => Some(Err(e)),
            }
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn pid(&self) -> PID {
        self.header().pid()
    }

    #[inline(always)]
    pub fn cc(&self) -> u8 {
        self.header().cc()
    }

    #[inline(always)]
    pub fn pcr(&self) -> Result<Option<PCR<'buf>>> {
        self.adaptation()
            .and_then(|res| match res {
                Ok(adapt) => adapt.pcr().and_then(|pcr| Some(Ok(pcr))),
                Err(e) => Some(Err(e)),
            })
            .transpose()
    }

    #[inline(always)]
    pub fn pat(&self) -> Result<Option<TablePAT>> {
        let header = self.header();

        if !header.got_payload() {
            return Ok(None);
        }

        let res = match self.pid() {
            PID::PAT => {
                let mut pos = self.buf_pos_payload();

                // TODO: remove
                pos += 3; // table-header;
                pos += 5; // syntax-section;

                // TODO: move to macro? or optional-result crate
                match self.buf_try_seek(pos) {
                    Ok(buf) => Some(Ok(TablePAT::new(buf))),
                    Err(e) => Some(Err(e)),
                }
            }
            _ => None,
        };

        res.transpose()
    }
}
