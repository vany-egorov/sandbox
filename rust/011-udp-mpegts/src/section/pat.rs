use super::traits::*;
use crate::result::Result;
use std::fmt;

/// ISO/IEC 13818-1
///
/// Program association Table
pub struct PAT<'buf> {
    buf: &'buf [u8],
}

impl<'buf> PAT<'buf> {
    const HEADER_FULL_SZ: usize = HEADER_SZ + SYNTAX_SECTION_SZ;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> PAT<'buf> {
        PAT { buf }
    }

    // #[inline(always)]
    // fn try_new(buf: &'buf [u8]) -> Result<PAT<'buf>> {
    //     let s = PAT::new(buf);
    //     s.validate()?;
    //     Ok(s)
    // }

    // #[inline(always)]
    // pub fn validate(&self) -> Result<()> {
    //     if self.buf.len() < Self::HEADER_FULL_SZ {
    //         Err(Error::new(ErrorKind::Buf(
    //             self.buf.len(),
    //             Self::HEADER_FULL_SZ,
    //         )))
    //     } else if self.buf.len() < self.sz() {
    //         Err(Error::new(ErrorKind::Buf(self.buf.len(), self.sz())))
    //     } else {
    //         Ok(())
    //     }
    // }

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
    pub fn programs(&self) -> Cursor<'buf, Program> {
        Cursor::new(self.buf_programs())
    }

    pub fn first_program_map_pid(&self) -> Option<u16> {
        self.programs().next().and_then(|res| match res {
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
            ":PAT (:table-id {:?} :section-length {})",
            self.table_id(),
            self.section_length(),
        )?;

        write!(f, "\n  :programs")?;
        for p in self.programs().filter_map(Result::ok) {
            write!(f, "\n    ")?;
            p.fmt(f)?;
        }

        Ok(())
    }
}

#[derive(Debug)]
pub enum PID {
    Network(u16),

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

    #[inline(always)]
    pub fn number(&self) -> u16 {
        (u16::from(self.buf[0]) << 8) | u16::from(self.buf[1])
    }

    #[inline(always)]
    pub fn pid_raw(&self) -> u16 {
        (u16::from(self.buf[2] & 0b0001_1111) << 8) | u16::from(self.buf[3])
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
            ":program (:number {:?} :pid {:?}/0x{:02X})",
            self.number(),
            self.pid(),
            self.pid_raw(),
        )
    }
}