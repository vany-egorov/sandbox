use nom;
use std::borrow::Cow;
use std::convert::Into;
use std::error::Error as StdError;
use std::fmt;
use std::io::Error as IoError;
use std::result::Result as StdResult;
use std::str::Utf8Error;

pub type Result<T> = StdResult<T, Error>;

macro_rules! from {
    ($src:path, $dst:path) => {
        impl From<$src> for Error {
            fn from(err: $src) -> Error {
                Error::new($dst(err))
            }
        }
    };
}

#[derive(Debug)]
pub enum Kind {
    InputUrlMissingHost,
    InputUrlHostMustBeDomain,
    UDPSocketBind,
    UDPSocketJoin,
    Io(IoError),
    Encoding(Utf8Error),
    Nom,        // TODO: rewrite
    SyncPoison, // TODO: rewrite

    TSSyncByte(u8),
    TSBuf(usize, usize),

    Unknown(Box<StdError + Send + Sync>),
}

pub struct Error {
    pub kind: Kind,
    pub details: Option<Cow<'static, str>>,
}

impl Error {
    pub fn new(kind: Kind) -> Error
    {
        Error {
            kind: kind,
            details: None,
        }
    }

    pub fn new_with_details<I>(kind: Kind, details: I) -> Error
    where
        I: Into<Cow<'static, str>>,
    {
        Error {
            kind: kind,
            details: Some(details.into()),
        }
    }

    // pub fn into_box(self) -> Box<StdError> {
    //     match self.kind {
    //         Kind::Unknown(err) => err,
    //         _ => Box::new(self),
    //     }
    // }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.details.as_ref() {
            Some(details) => {
                write!(
                    f,
                    "Error <{:?}>: \"{}\" / \"{}\"",
                    self.kind,
                    self.description(),
                    details,
                )
            },
            None => {
                write!(
                    f,
                    "Error <{:?}>: \"{}\"",
                    self.kind,
                    self.description(),
                )
            },
        }
    }
}

impl StdError for Error {
    fn description(&self) -> &str {
        match self.kind {
            Kind::InputUrlMissingHost => "missing host inside input URL",
            Kind::InputUrlHostMustBeDomain => "provided host must be valid domain",
            Kind::UDPSocketBind => "udp-socket bind failed",
            Kind::UDPSocketJoin => "udp-socket join failed",
            Kind::Encoding(ref err) => err.description(),
            Kind::Io(ref err) => err.description(),
            Kind::Nom => "nom parser error",
            Kind::SyncPoison => "sync lock/condvar poison error",

            Kind::TSSyncByte(..) => "ts: expected sync byte as first element",
            Kind::TSBuf(..) => "ts: buffer is too small, more data required",

            Kind::Unknown(ref err) => err.description(),
        }
    }

    fn cause(&self) -> Option<&StdError> {
        match self.kind {
            Kind::InputUrlMissingHost => None,
            Kind::InputUrlHostMustBeDomain => None,
            Kind::UDPSocketBind => None,
            Kind::UDPSocketJoin => None,
            Kind::Encoding(ref err) => Some(err),
            Kind::Io(ref err) => Some(err),
            Kind::Nom => None,
            Kind::SyncPoison => None,

            Kind::Unknown(ref err) => Some(err.as_ref()),

            _ => None,
        }
    }
}

from!(Utf8Error, Kind::Encoding);
from!(IoError, Kind::Io);

impl From<nom::Err<&[u8]>> for Error {
    fn from(err: nom::Err<&[u8]>) -> Error {
        match err {
            _ => Error::new(Kind::Nom),
        }
    }
}

impl<B> From<Box<B>> for Error
where
    B: StdError + Send + Sync + 'static,
{
    fn from(err: Box<B>) -> Error {
        Error::new(Kind::Unknown(err))
    }
}
