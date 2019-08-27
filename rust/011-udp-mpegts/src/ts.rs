use std::fmt;
use std::time::Duration;

use crate::duration_fmt::DurationFmt;
use crate::error::{Error, Kind as ErrorKind};
use crate::pid::PID;
use crate::rational;
use crate::rational::Rational;
use crate::result::Result;
use crate::section::{PAT, PMT, SDT};

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
            ":PCR (:raw {:08X}:{:04X} :v(27MHz) {} :duration {})",
            self.base(),
            self.ext(),
            self.value(),
            DurationFmt::from(self)
        )
    }
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

    #[inline(always)]
    fn field_length(&self) -> usize {
        self.buf[0] as usize
    }

    #[inline(always)]
    #[allow(dead_code)]
    fn discontinuity_indicator(&self) -> bool {
        (self.buf[1] & 0b1000_0000) != 0
    }

    #[inline(always)]
    #[allow(dead_code)]
    fn random_access_indicator(&self) -> bool {
        (self.buf[1] & 0b0100_0000) != 0
    }

    #[inline(always)]
    pub fn elementary_stream_priority_indicator(&self) -> bool {
        (self.buf[1] & 0b0010_0000) != 0
    }

    /// PCR field is present?
    #[inline(always)]
    fn pcr_flag(&self) -> bool {
        (self.buf[1] & 0b0001_0000) != 0
    }

    /// OPCR field is present?
    #[inline(always)]
    pub fn opcr_flag(&self) -> bool {
        (self.buf[1] & 0b0000_1000) != 0
    }

    /// splice countdown field is present?
    #[inline(always)]
    pub fn splicing_point_flag(&self) -> bool {
        (self.buf[1] & 0b0000_0100) != 0
    }

    /// transport private data is present?
    #[inline(always)]
    pub fn transport_private_data_flag(&self) -> bool {
        (self.buf[1] & 0b0000_0010) != 0
    }

    /// transport private data is present?
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

    #[inline(always)]
    #[allow(dead_code)]
    fn tei(&self) -> bool {
        (self.buf[1] & 0b1000_0000) != 0
    }

    #[inline(always)]
    pub fn pusi(&self) -> bool {
        (self.buf[1] & 0b0100_0000) != 0
    }

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
    pub const SZ: usize = 188;
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
    pub fn pusi(&self) -> bool {
        self.header().pusi()
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

    // TODO: generic pmt, pat method
    #[inline(always)]
    pub fn pat(&self) -> Result<Option<PAT>> {
        let header = self.header();

        if !header.got_payload() {
            return Ok(None);
        }

        let res = if self.pid() == PID::PAT {
            // TODO: move to macro? or optional-result crate
            match self.buf_try_seek(self.buf_pos_payload()) {
                Ok(buf) => Some(Ok(PAT::new(buf))),
                Err(e) => Some(Err(e)),
            }
        } else {
            None
        };

        res.transpose()
    }

    #[inline(always)]
    pub fn pmt(&self, pid: u16) -> Result<Option<PMT>> {
        let header = self.header();

        if !header.got_payload() {
            return Ok(None);
        }

        let res = if u16::from(self.pid()) == pid {
            // TODO: move to macro? or optional-result crate
            match self.buf_try_seek(self.buf_pos_payload()) {
                Ok(buf) => Some(Ok(PMT::new(buf))),
                Err(e) => Some(Err(e)),
            }
        } else {
            None
        };

        res.transpose()
    }

    #[inline(always)]
    pub fn sdt(&self) -> Result<Option<SDT>> {
        let header = self.header();

        if !header.got_payload() {
            return Ok(None);
        }

        let res = if self.pid() == PID::SDT {
            // TODO: move to macro? or optional-result crate
            match self.buf_try_seek(self.buf_pos_payload()) {
                Ok(buf) => Some(Ok(SDT::new(buf))),
                Err(e) => Some(Err(e)),
            }
        } else {
            None
        };

        res.transpose()
    }

    #[inline(always)]
    pub fn eit(&self) -> Result<Option<&'buf [u8]>> {
        let header = self.header();

        if !header.got_payload() {
            return Ok(None);
        }

        let res = if self.pid() == PID::EIT {
            // TODO: move to macro? or optional-result crate
            match self.buf_try_seek(self.buf_pos_payload()) {
                Ok(buf) => Some(Ok(buf)),
                Err(e) => Some(Err(e)),
            }
        } else {
            None
        };

        res.transpose()
    }
}
