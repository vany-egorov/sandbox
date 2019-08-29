use std::fmt;

use crate::error::{Error, Kind as ErrorKind};
use crate::result::Result;

#[derive(Debug)]
pub enum StreamID {
    Raw(u8),
}

impl From<u8> for StreamID {
    fn from(d: u8) -> Self {
        StreamID::Raw(d)
    }
}

impl From<StreamID> for u8 {
    fn from(sid: StreamID) -> u8 {
        let StreamID::Raw(d) = sid;
        d
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum PtsDtsFlag {
    No,
    Pts,
    PtsDts,
    Forbidden,
}

impl From<u8> for PtsDtsFlag {
    #[inline(always)]
    fn from(d: u8) -> Self {
        match d {
            0b00 => PtsDtsFlag::No,
            0b10 => PtsDtsFlag::Pts,
            0b11 => PtsDtsFlag::PtsDts,

            _ => PtsDtsFlag::Forbidden,
        }
    }
}

/// http://dvd.sourceforge.net/dvdinfo/pes-hdr.html
pub struct PES<'buf> {
    buf: &'buf [u8],
}

impl<'buf> PES<'buf> {
    const HEADER_SZ: usize = 6;
    const START_CODE: u32 = 0x0000_0001;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> PES<'buf> {
        PES { buf }
    }

    #[inline(always)]
    pub fn try_new(buf: &'buf [u8]) -> Result<PES<'buf>> {
        let p = PES::new(buf);
        p.validate()?;
        Ok(p)
    }

    #[inline(always)]
    pub fn validate(&self) -> Result<()> {
        if self.buf.len() < Self::HEADER_SZ {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::HEADER_SZ)))
        } else if self.start_code() != Self::START_CODE {
            Err(Error::new(ErrorKind::PESStartCode(self.start_code())))
        } else {
            Ok(())
        }
    }

    #[inline(always)]
    fn start_code(&self) -> u32 {
        (u32::from(self.buf[0]) << 16) | (u32::from(self.buf[1]) << 8) | u32::from(self.buf[2])
    }

    #[inline(always)]
    fn stream_id(&self) -> StreamID {
        StreamID::from(self.buf[3])
    }

    #[inline(always)]
    fn packet_length(&self) -> u16 {
        u16::from(self.buf[4]) << 8 | u16::from(self.buf[5])
    }

    #[inline(always)]
    fn pes_header_data_length(&self) -> u8 {
        self.buf[8]
    }
}

impl<'buf> fmt::Debug for PES<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, ":PES (:stream-id {:?} :packet-len {} :header-len {})",
            self.stream_id(), self.packet_length(), self.pes_header_data_length())
    }
}
