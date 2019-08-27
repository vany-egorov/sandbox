use std::error::Error as StdError;
use std::fmt;

#[derive(Debug)]
pub enum Kind {
    SyncByte(u8),
    Buf(usize, usize),
    AnnexA2EmptyBuf,
    AnnexA2UnsupportedEncoding,
    AnnexA2Decode,
    AnnexA2TableA3Unexpected(u8),
    AnnexA2TableA4Buf(usize, usize),
    AnnexA2TableA4Unexpected(u8),
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
            },

            Kind::AnnexA2TableA3Unexpected(b) => write!(f, " (:got 0x{:02X})", b)?,
            Kind::AnnexA2TableA4Buf(actual, expected) => {
                write!(f, " (:sz-actual {} :sz-expected {})", actual, expected)?
            },
            Kind::AnnexA2TableA4Unexpected(b) => write!(f, " (:got 0x{:02X})", b)?,
            _ => {},
        }

        write!(f, "))")
    }
}

impl StdError for Error {
    fn description(&self) -> &str {
        match self.0 {
            Kind::SyncByte(..) => "expected sync byte as first element",
            Kind::Buf(..) => "buffer is too small, more data required",

            Kind::AnnexA2UnsupportedEncoding => "(annex-a2) unsupported encoding",
            Kind::AnnexA2Decode => "(annex-a2) decode error",
            Kind::AnnexA2EmptyBuf => "(annex-a2 parse) got empty character buffer",
            Kind::AnnexA2TableA3Unexpected(..) => "(annex-a2 table-a3 parse) unexpected value",
            Kind::AnnexA2TableA4Buf(..) => "(annex-a2 table-a4 parse) buffer is too small, more data required",
            Kind::AnnexA2TableA4Unexpected(..) => "(annex-a2 table-a4 parse) unexpected value",
        }
    }

    fn cause(&self) -> Option<&dyn StdError> {
        None
    }
}
