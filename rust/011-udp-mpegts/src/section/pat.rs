use std::fmt;
use super::traits::*;
use crate::result::Result;

/// ISO/IEC 13818-1
///
/// Program association Table
///
/// Stream Type: ITU-T Rec. H.222.0 | ISO/IEC 13818-1
///
/// Associates Program Number and Program Map Table PID
///
/// The Program Association Table provides the correspondence
/// between a program_number and the PID value of the
/// Transport Stream packets which carry the program definition.
/// The program_number is the numeric label associated with
/// a program.
pub struct PAT<'buf> {
    buf: &'buf [u8],
}

impl<'buf> PAT<'buf> {
    const HEADER_FULL_SZ: usize = HEADER_SZ + SYNTAX_SECTION_SZ;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> PAT<'buf> {
        PAT { buf }
    }

    /// slice buf
    #[inline(always)]
    fn buf_programs(&self) -> &'buf [u8] {
        let lft = Self::HEADER_FULL_SZ;
        let mut rght = HEADER_SZ + (self.section_length() as usize);

        if rght >= self.buf.len() {
            rght = self.buf.len();
        }

        rght -= CRC32_SZ;

        &self.buf[lft..rght]
    }

    #[inline(always)]
    pub fn programs(&self) -> Rows<'buf, Program> {
        Rows::new(self.buf_programs())
    }

    pub fn first_program_map_pid(&self) -> Option<u16> {
        self
            .programs()
            .next()
            .and_then(|res| match res {
                Ok(p) => match p.pid() {
                    PID::ProgramMap(v) => Some(v),
                    _ => None,
                },
                _ => None,
            })
    }
}

impl<'buf> Bufer<'buf> for PAT<'buf> {
    fn buf(&self) -> &'buf [u8] {
        self.buf
    }
}

impl<'buf> WithHeader<'buf> for PAT<'buf> {}
impl<'buf> WithSyntaxSection<'buf> for PAT<'buf> {}
impl<'buf> WithCRC32<'buf> for PAT<'buf> {}

impl<'buf> fmt::Debug for PAT<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:PAT (:table-id {:?} :section-length {}",
            self.table_id(), self.section_length(),
        )?;

        write!(f, "\n  :programs")?;
        for p in self.programs().filter_map(Result::ok) {
            write!(f, "\n    ")?;
            p.fmt(f)?;
        }

        write!(f, "))")
    }
}

#[derive(Debug)]
pub enum PID {
    /// The network_PID is a 13-bit field, which is used only
    /// in conjunction with the value of the program_number
    /// set to 0x0000, specifies the PID of the Transport Stream
    /// packets which shall contain the Network Information Table.
    /// The value of the network_PID field is defined by the user,
    /// but shall only take values as specified in Table 2-3.
    /// The presence of the network_PID is optional.
    Network(u16),

    /// The program_map_PID is a 13-bit field specifying the PID
    /// of the Transport Stream packets which shall contain the
    /// program_map_section applicable for the program as specified by
    /// the program_number. No program_number shall have more than one
    /// program_map_PID assignment. The value of the program_map_PID is
    /// defined by the user, but shall only take values as specified
    /// in Table 2-3.
    ProgramMap(u16),
}

pub struct Program<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Program<'buf> {
    const SZ: usize = 4;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Program<'buf> {
        Program { buf }
    }

    /// Program_number is a 16-bit field. It specifies the program
    /// to which the program_map_PID is applicable.
    /// When set to 0x0000, then the following PID reference
    /// shall be the network PID. For all other cases the value
    /// of this field is user defined. This field shall not take any
    /// single value more than once within one version of the
    /// Program Association Table.
    #[inline(always)]
    pub fn number(&self) -> u16 {
        ((self.buf[0] as u16) << 8) | self.buf[1] as u16
    }

    #[inline(always)]
    pub fn pid_raw(&self) -> u16 {
        (((self.buf[2] & 0b0001_1111) as u16) << 8) | self.buf[3] as u16
    }

    #[inline(always)]
    pub fn pid(&self) -> PID {
        match self.number() {
            0 => PID::Network(self.pid_raw()),
            _ => PID::ProgramMap(self.pid_raw()),
        }
    }
}

impl<'buf> Szer for Program<'buf> {
    #[inline(always)]
    fn sz(&self) -> usize {
        Program::SZ
    }
}

impl<'buf> TryNewer<'buf> for Program<'buf> {
    #[inline(always)]
    fn try_new(buf: &'buf [u8]) -> Result<Program<'buf>> {
        let p = Program::new(buf);
        Ok(p)
    }
}

impl<'buf> fmt::Debug for Program<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "(:program (:number {:?} :pid {:?}/0x{:02X}))",
            self.number(), self.pid(), self.pid_raw(),
        )
    }
}
