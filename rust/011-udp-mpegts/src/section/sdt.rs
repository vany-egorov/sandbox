use std::fmt;
use super::traits::*;
use crate::result::Result;
use crate::descriptor::Descriptor;

/// ETSI EN 300 468 V1.15.1
///
/// Service Description Table
///
/// the SDT contains data describing the services in the system
/// e.g. names of services, the service provider, etc.
///
/// The SDT shall be segmented into service_description_sections
/// using the syntax of table 5. Any sections forming part of an SDT
/// shall be transmitted in TS packets with a PID value of 0x0011.
/// Any sections of the SDT which describe the actual TS (that is,
/// the TS containing the SDT) shall have the table_id value 0x42
/// with the same table_id_extension (transport_stream_id) and with
/// the same original_network_id. Any sections of an SDT which refer
/// to a TS other than the actual TS shall take a table_id value of 0x46.
pub struct SDT<'buf> {
    buf: &'buf [u8],
}

impl<'buf> SDT<'buf> {
    const HEADER_SPECIFIC_SZ: usize = 3;
    const HEADER_FULL_SZ: usize = HEADER_SZ + SYNTAX_SECTION_SZ + Self::HEADER_SPECIFIC_SZ;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> SDT<'buf> {
        SDT { buf }
    }

    /// seek
    #[inline(always)]
    fn buf_streams(&self) -> &'buf [u8] {
        let lft = Self::HEADER_FULL_SZ;
        let mut rght = HEADER_SZ + (self.section_length() as usize);

        if rght >= self.buf.len() {
            rght = self.buf.len();
        }

        rght -= CRC32_SZ;

        &self.buf[lft..rght]
    }

    #[inline(always)]
    pub fn streams(&self) -> Cursor<'buf, Stream> {
        Cursor::new(self.buf_streams())
    }
}

trait WithSDTHeaderSpecific<'buf>: Bufer<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        &self.buf()[HEADER_SZ + SYNTAX_SECTION_SZ..]
    }

    /// This 16-bit field gives the label identifying
    /// the network_id of the originating delivery system.
    #[inline(always)]
    fn original_network_id(&self) -> u16 {
        (self.b()[0] as u16) | (self.b()[1] as u16)
    }
}

impl<'buf> Bufer<'buf> for SDT<'buf> {
    fn buf(&self) -> &'buf [u8] {
        self.buf
    }
}

impl<'buf> WithHeader<'buf> for SDT<'buf> {}
impl<'buf> WithSyntaxSection<'buf> for SDT<'buf> {}
impl<'buf> WithSDTHeaderSpecific<'buf> for SDT<'buf> {}
impl<'buf> WithCRC32<'buf> for SDT<'buf> {}

impl<'buf> fmt::Debug for SDT<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:SDT (:table-id {:?} :section-length {} :section {}/{}",
            self.table_id(),
            self.section_length(),
            self.section_number(), self.last_section_number(),
        )?;

        write!(f, "\n  :streams")?;
        for p in self.streams().filter_map(Result::ok) {
            write!(f, "\n    ")?;
            p.fmt(f)?;
        }

        write!(f, "))")
    }
}

pub struct Stream<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Stream<'buf> {
    const HEADER_SZ: usize = 5;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Stream<'buf> {
        Stream { buf }
    }

    /// This is a 16-bit field which serves as a label to identify
    /// this service from any other service within the TS.
    /// The service_id is the same as the program_number
    /// in the corresponding program_map_section.
    #[inline(always)]
    pub fn service_id(&self) -> u16 {
        (self.buf[0] as u16) | (self.buf[1] as u16)
    }

    /// This is a 1-bit field which when set to "1" indicates
    /// that EIT schedule information for the service
    /// is present in the current TS, see ETSI TS 101 211 [i.2]
    /// for information on maximum time interval between occurrences
    /// of an EIT schedule sub_table). If the flag is set to 0 then
    /// the EIT schedule information for the service should not be
    /// present in the TS.
    #[inline(always)]
    pub fn eit_schedule_flag(&self) -> bool {
        ((self.buf[2] & 0b0000_0010) >> 1) != 0
    }

    /// This is a 1-bit field which when set to "1" indicates
    /// that EIT_present_following information for the service is present
    /// in the current TS, see ETSI TS 101 211 [i.2] for information
    /// on maximum time interval between occurrences of an EIT
    /// present/following sub_table. If the flag is set to 0 then
    /// the EIT present/following information for the service
    /// should not be present in the TS.
    #[inline(always)]
    pub fn eit_present_following_flag(&self) -> bool {
        (self.buf[2] & 0b0000_0001) != 0
    }

    // TODO: add enum
    #[inline(always)]
    pub fn running_status(&self) -> u8 {
        (self.buf[3] & 0b1110_0000) >> 5
    }

    /// This 1-bit field, when set to "0" indicates that all
    /// the component streams of the service are not
    /// scrambled. When set to "1" it indicates that access
    /// to one or more streams may be controlled by a CA system.
    #[inline(always)]
    pub fn free_ca_mode(&self) -> bool {
        (self.buf[3] & 0b0001_0000) != 0
    }

    /// This 12-bit field gives the total length in bytes
    /// of the following descriptors.
    #[inline(always)]
    pub fn descriptors_loop_length(&self) -> u16 {
        (((self.buf[3] & 0b0000_1111) as u16) << 8) | (self.buf[4] as u16)
    }

    /// seek
    #[inline(always)]
    fn buf_descriptors(&self) -> &'buf [u8] {
        let lft = Self::HEADER_SZ;
        let mut rght = lft + (self.descriptors_loop_length() as usize);

        if rght >= self.buf.len() {
            rght = self.buf.len();
        }

        &self.buf[lft..rght]
    }

    #[inline(always)]
    pub fn descriptors(&self) -> Option<Cursor<'buf, Descriptor>> {
        if self.descriptors_loop_length() != 0 {
            Some(Cursor::new(self.buf_descriptors()))
        } else {
            None
        }
    }
}

impl<'buf> Szer for Stream<'buf> {
    #[inline(always)]
    fn sz(&self) -> usize {
        Self::HEADER_SZ + (self.descriptors_loop_length() as usize)
    }
}

impl<'buf> TryNewer<'buf> for Stream<'buf> {
    #[inline(always)]
    fn try_new(buf: &'buf [u8]) -> Result<Stream<'buf>> {
        let s = Stream::new(buf);
        Ok(s)
    }
}

impl<'buf> fmt::Debug for Stream<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:stream (:service-id {:?} :running-status {:?}",
            self.service_id(), self.running_status(),
        )?;

        write!(f, "\n      :descriptors")?;
        match self.descriptors() {
          Some(descs)  => {
            for d in descs.filter_map(Result::ok) {
                write!(f, "\n        ")?;
                d.fmt(f)?;
            }
          },
          None => write!(f, " ~")?,
        }

        write!(f, "))")
    }
}
