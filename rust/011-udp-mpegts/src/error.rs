use std::error::Error as StdError;
use std::fmt;

#[derive(Debug)]
pub enum Kind {
    SyncByte(u8),
    Buf(usize, usize),
}

pub struct Error(Kind);

impl Error {
    pub fn new(kind: Kind) -> Error {
        Error(kind)
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            r#"(:error ({:?}) (:txt "{}""#,
            self.0,
            self.description()
        )?;

        match self.0 {
            Kind::SyncByte(b) => write!(f, " (:got 0x{:02X})", b)?,
            Kind::Buf(actual, expected) => {
                write!(f, " (:sz-actual {} :sz-expected {})", actual, expected)?
            }
        }

        write!(f, "))")
    }
}

impl StdError for Error {
    fn description(&self) -> &str {
        match self.0 {
            Kind::SyncByte(..) => "expected sync byte as first element",
            Kind::Buf(..) => "buffer is too small, more data required",
        }
    }

    fn cause(&self) -> Option<&dyn StdError> {
        None
    }
}
