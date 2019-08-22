use super::traits::*;
use std::fmt;

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
    #[allow(dead_code)]
    const HEADER_SPECIFIC_SZ: usize = 4;
    #[allow(dead_code)]
    const HEADER_FULL_SZ: usize = HEADER_SZ + SYNTAX_SECTION_SZ + Self::HEADER_SPECIFIC_SZ;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> PMT<'buf> {
        PMT { buf }
    }
}

trait WithPMTHeaderSpecific<'buf>: WithBuf<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        &self.buf()[HEADER_SZ+SYNTAX_SECTION_SZ..]
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

impl<'buf> WithBuf<'buf> for PMT<'buf> {
    /// borrow a reference to the underlying buffer
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
            self.table_id(), self.section_length(), self.pcr_pid(), self.program_info_length(),
        )?;

        write!(f, "))")
    }
}

#[allow(dead_code)]
pub struct Stream<'buf> {
    buf: &'buf [u8],
}

impl <'buf> Stream<'buf> {
    #[inline(always)]
    #[allow(dead_code)]
    pub fn new(buf: &'buf [u8]) -> Stream<'buf> {
        Stream { buf }
    }
}
