use std::fmt;
use super::traits::*;
use crate::result::Result;
use crate::error::{Error, Kind as ErrorKind};
use crate::descriptor::Descriptor;

/// ETSI EN 300 468 V1.15.1
///
/// Event Information Tabl
///
/// The EIT (see table 7) provides information in chronological
/// order regarding the events contained within each service.
/// Four classifications of EIT have been identified, distinguishable
/// by the use of different table_ids:
///
/// 1)actual TS, present/following event information = table_id = 0x4E;
/// 2) other TS, present/following event information = table_id = 0x4F;
/// 3) actual TS, event schedule information = table_id = 0x50 to 0x5F;
/// 4) other TS, event schedule information = table_id = 0x60 to 0x6F.
///
/// All EIT sub-tables for the actual Transport Stream shall have the
/// same transport_stream_id and original_network_id values.
///
/// The present/following table shall contain only information pertaining
/// to the present event and the chronologically following event carried
/// by a given service on either the actual TS or another TS, except in
/// the case of a Near Video On Demand (NVOD) reference service where it may
/// have more than two event descriptions. The EIT present/following table
/// is optional. Its presence or absence shall be signalled by setting the
/// EIT_present_following_flag in the SDT. The event schedule tables for
/// either the actual TS or other TSs, contain a list of events, in the form
/// of a schedule including events other than the present and following
/// events. The EIT schedule tables are optional. Their presence or absence
/// shall be signalled by setting the EIT_schedule_flag in the SDT. The event
/// information shall be chronologically ordered.
///
/// The EIT shall be segmented into event_information_sections using the
/// syntax of table 7. Any sections forming part of an EIT shall be
/// transmitted in TS packets with a PID value of 0x0012.
pub struct EIT<'buf> {
    buf: &'buf [u8],
}

impl<'buf> EIT<'buf> {
    const HEADER_SPECIFIC_SZ: usize = 6;
    const HEADER_FULL_SZ: usize = HEADER_SZ + SYNTAX_SECTION_SZ + Self::HEADER_SPECIFIC_SZ;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> EIT<'buf> {
        EIT { buf }
    }

    /// seek
    #[inline(always)]
    fn buf_events(&self) -> &'buf [u8] {
        let lft = Self::HEADER_FULL_SZ;
        let mut rght = HEADER_SZ + (self.section_length() as usize);

        if rght >= self.buf.len() {
            rght = self.buf.len();
        }

        rght -= CRC32_SZ;

        &self.buf[lft..rght]
    }

    #[inline(always)]
    pub fn events(&self) -> Cursor<'buf, Event> {
        Cursor::new(self.buf_events())
    }
}

trait WithEITHeaderSpecific<'buf>: Bufer<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        &self.buf()[HEADER_SZ + SYNTAX_SECTION_SZ..]
    }

    /// This is a 16-bit field which serves as a label for
    /// identification of the TS, about which the EIT
    /// informs, from any other multiplex within the delivery system.
    #[inline(always)]
    fn transport_stream_id(&self) -> u16 {
        (self.b()[0] as u16) | (self.b()[1] as u16)
    }

    /// This 16-bit field gives the label identifying the
    /// network_id of the originating delivery system.
    #[inline(always)]
    fn original_network_id(&self) -> u16 {
        (self.b()[2] as u16) | (self.b()[3] as u16)
    }

    /// This 8-bit field specifies the number of the last
    /// section of this segment of the sub_table. For sub_tables
    /// which are not segmented, this field shall be set to the
    /// same value as the last_section_number field.
    #[inline(always)]
    fn segment_last_section_number(&self) -> u8 {
        self.b()[4]
    }

    /// This 8-bit field identifies the last table_id used (see table 2).
    /// For EIT present/following tables, this field shall be set to the
    /// same value as the table_id field. For EIT schedule tables with
    /// table_id in the range 0x50 to 0x5F, this field shall be set to
    /// the largest table_id transmitted in this range for this service.
    /// For EIT schedule tables with table_id in the range 0x60 to 0x6F,
    /// this field shall be set to the largest table_id transmitted in this
    /// range for this service.
    ///
    /// NOTE: This implies that the value of last_table_id may be
    ///       different for each service.
    #[inline(always)]
    fn last_table_id(&self) -> u8 {
        self.b()[5]
    }
}

impl<'buf> Bufer<'buf> for EIT<'buf> {
    fn buf(&self) -> &'buf [u8] {
        self.buf
    }
}

impl<'buf> WithHeader<'buf> for EIT<'buf> {}
impl<'buf> WithSyntaxSection<'buf> for EIT<'buf> {}
impl<'buf> WithEITHeaderSpecific<'buf> for EIT<'buf> {}
impl<'buf> WithCRC32<'buf> for EIT<'buf> {}

impl<'buf> fmt::Debug for EIT<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:EIT (:table-id {:?} :section-length {} :section {}/{}",
            self.table_id(),
            self.section_length(),
            self.section_number(), self.last_section_number(),
        )?;

        write!(f, "\n  :events")?;
        for rese in self.events() {
            write!(f, "\n    ")?;
            match rese  {
                Ok(e) => {
                    e.fmt(f)?;
                },
                Err(err) => {
                    write!(f, "error parse EIT event: {}", err)?;
                }
            }
        }

        write!(f, "))")
    }
}

pub struct Event<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Event<'buf> {
    const HEADER_SZ: usize = 12;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Event<'buf> {
        Event { buf }
    }

    #[inline(always)]
    pub fn validate(&self) -> Result<()> {
        if self.buf.len() < Self::HEADER_SZ {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::HEADER_SZ)))
        } else if self.buf.len() < self.sz() {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), self.sz())))
        } else {
            Ok(())
        }
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

    /// This 12-bit field gives the total length in bytes
    /// of the following descriptors.
    #[inline(always)]
    pub fn descriptors_loop_length(&self) -> u16 {
        (((self.buf[10] & 0b0000_1111) as u16) << 8) | (self.buf[11] as u16)
    }
}

impl<'buf> Szer for Event<'buf> {
    #[inline(always)]
    fn sz(&self) -> usize {
        Self::HEADER_SZ + (self.descriptors_loop_length() as usize)
    }
}

impl<'buf> TryNewer<'buf> for Event<'buf> {
    #[inline(always)]
    fn try_new(buf: &'buf [u8]) -> Result<Event<'buf>> {
        let s = Event::new(buf);
        s.validate()?;
        Ok(s)
    }
}

impl<'buf> fmt::Debug for Event<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:event (",
        )?;

        write!(f, "\n      :descriptors")?;
        match self.descriptors() {
          Some(descs)  => {
            for resd in descs {
                write!(f, "\n        ")?;
                match resd {
                    Ok(d) => {
                        d.fmt(f)?;
                    },
                    Err(err) => {
                        write!(f, "error parse descriptor: {}", err)?;
                    }
                }
            }
          },
          None => write!(f, " ~")?,
        }

        write!(f, "))")
    }
}
