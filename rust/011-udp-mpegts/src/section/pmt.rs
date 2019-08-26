use std::fmt;
use super::traits::*;
use crate::result::Result;
use crate::stream_type::StreamType;
use crate::descriptor::Descriptor;

/// ISO/IEC 13818-1
///
/// Program Map Table
///
/// Stream Type: ITU-T Rec. H.222.0 | ISO/IEC 13818-1
///
/// Specifies PID values for components of one or more programs
///
/// The Program Map Table provides the mappings between program
/// numbers and the program elements that comprise them.
/// A single instance of such a mapping is referred to as a
/// "program definition". The program map table is the complete
/// collection of all program definitions for a Transport Stream.
/// This table shall be transmitted in packets, the PID values
/// of which are selected by the encoder. More than one PID value
/// may be used, if desired. The table is contained in one or
/// more sections with the following syntax. It may be segmented
/// to occupy multiple sections. In each section, the section number
/// field shall be set to zero. Sections are identified by the
/// program_number field.
pub struct PMT<'buf> {
    buf: &'buf [u8],
}

impl<'buf> PMT<'buf> {
    const HEADER_SPECIFIC_SZ: usize = 4;
    const HEADER_FULL_SZ: usize = HEADER_SZ + SYNTAX_SECTION_SZ + Self::HEADER_SPECIFIC_SZ;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> PMT<'buf> {
        PMT { buf }
    }

    /// seek
    #[inline(always)]
    fn buf_streams(&self) -> &'buf [u8] {
        let lft = Self::HEADER_FULL_SZ + (self.program_info_length() as usize);
        let mut rght = HEADER_SZ + (self.section_length() as usize);

        if rght >= self.buf.len() {
            rght = self.buf.len();
        }

        rght -= CRC32_SZ;

        &self.buf[lft..rght]
    }

    /// seek
    #[inline(always)]
    fn buf_descriptors(&self) -> &'buf [u8] {
        let lft = Self::HEADER_FULL_SZ;
        let rght = Self::HEADER_FULL_SZ + (self.program_info_length() as usize);

        &self.buf[lft..rght]
    }

    #[inline(always)]
    pub fn descriptors(&self) -> Option<Cursor<'buf, Descriptor>> {
        if self.program_info_length() != 0 {
            Some(Cursor::new(self.buf_descriptors()))
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn streams(&self) -> Cursor<'buf, Stream> {
        Cursor::new(self.buf_streams())
    }
}

trait WithPMTHeaderSpecific<'buf>: Bufer<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        &self.buf()[HEADER_SZ + SYNTAX_SECTION_SZ..]
    }

    /// This is a 13-bit field indicating the PID of
    /// the Transport Stream packets which shall contain the PCR fields
    /// valid for the program specified by program_number.
    /// If no PCR is associated with a program definition for private
    /// streams, then this field shall take the value of 0x1FFF.
    /// Refer to the semantic definition of PCR in 2.4.3.5 and Table 2-3
    /// for restrictions on the choice of PCR_PID value.
    #[inline(always)]
    fn pcr_pid(&self) -> u16 {
        ((self.b()[0] & 0b0001_1111) as u16) | (self.b()[1] as u16)
    }

    /// This is a 12-bit field, the first two bits of which shall be '00'.
    /// The remaining 10 bits specify the number of bytes of the descriptors
    /// immediately following the program_info_length field.
    #[inline(always)]
    fn program_info_length(&self) -> u16 {
        ((self.b()[2] & 0b0000_1111) as u16) | (self.b()[3] as u16)
    }
}

impl<'buf> Bufer<'buf> for PMT<'buf> {
    fn buf(&self) -> &'buf [u8] {
        self.buf
    }
}

impl<'buf> WithHeader<'buf> for PMT<'buf> {}
impl<'buf> WithSyntaxSection<'buf> for PMT<'buf> {}
impl<'buf> WithPMTHeaderSpecific<'buf> for PMT<'buf> {}
impl<'buf> WithCRC32<'buf> for PMT<'buf> {}

impl<'buf> fmt::Debug for PMT<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:PMT (:table-id {:?} :section-length {} :pcr-pid {} :program-info-length {}",
            self.table_id(),
            self.section_length(),
            self.pcr_pid(),
            self.program_info_length(),
        )?;

        write!(f, "\n  :descriptors")?;
        match self.descriptors() {
          Some(descs)  => {
            for d in descs.filter_map(Result::ok) {
                write!(f, "\n    ")?;
                d.fmt(f)?;
            }
          },
          None => write!(f, " ~")?,
        }

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

    /// This is an 8-bit field specifying the type of program
    /// element carried within the packets with the PID whose value
    /// is specified by the elementary_PID. The values of stream_type
    /// are specified in Table 2-29.
    ///
    /// NOTE – An ITU-T Rec. H.222.0 | ISO/IEC 13818-1 auxiliary stream
    /// is available for data types defined by this Specification, other than audio,
    /// video, and DSM CC, such as Program Stream Directory and Program Stream Map.
    #[inline(always)]
    fn stream_type(&self) -> StreamType {
        StreamType::from(self.buf[0])
    }

    /// This is a 13-bit field specifying the PID of the Transport Stream
    /// packets which carry the associated program element.
    #[inline(always)]
    fn pid(&self) -> u16 {
        (((self.buf[1] & 0b0001_1111) as u16) << 8) | (self.buf[2] as u16)
    }

    /// This is a 12-bit field, the first two bits of which shall be '00'.
    /// The remaining 10 bits specify the number of bytes of the descriptors
    /// of the associated program element immediately following
    /// the ES_info_length field.
    #[inline(always)]
    fn es_info_length(&self) -> u16 {
        (((self.buf[3] & 0b0000_1111) as u16) << 8) | (self.buf[4] as u16)
    }

    /// seek
    #[inline(always)]
    fn buf_descriptors(&self) -> &'buf [u8] {
        let lft = Self::HEADER_SZ;
        let mut rght = lft + (self.es_info_length() as usize);

        if rght >= self.buf.len() {
            rght = self.buf.len();
        }

        &self.buf[lft..rght]
    }

    #[inline(always)]
    pub fn descriptors(&self) -> Option<Cursor<'buf, Descriptor>> {
        if self.es_info_length() != 0 {
            Some(Cursor::new(self.buf_descriptors()))
        } else {
            None
        }
    }
}

impl<'buf> Szer for Stream<'buf> {
    #[inline(always)]
    fn sz(&self) -> usize {
        Self::HEADER_SZ + (self.es_info_length() as usize)
    }
}

impl<'buf> TryNewer<'buf> for Stream<'buf> {
    #[inline(always)]
    fn try_new(buf: &'buf [u8]) -> Result<Stream<'buf>> {
        let p = Stream::new(buf);
        Ok(p)
    }
}

impl<'buf> fmt::Debug for Stream<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:stream (:pid {:?} :stream-type {:?}",
            self.pid(), self.stream_type()
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
