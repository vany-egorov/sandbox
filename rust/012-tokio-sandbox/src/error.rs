use std::fmt;
use std::borrow::Cow;
use std::convert::Into;
use std::str::Utf8Error;
use std::io::Error as IoError;
use std::error::Error as StdError;
use std::result::Result as StdResult;


pub type Result<T> = StdResult<T, Error>;


macro_rules! from {
    ($src:path, $dst:path) => {
        impl From<$src> for Error {
            fn from(err: $src) -> Error {
                Error::new($dst(err), "")
            }
        }
    }
}


#[derive(Debug)]
pub enum Kind {
    // InputUrlMissingHost,
    // InputUrlHostMustBeIpv4,
    Io(IoError),
    Encoding(Utf8Error),
}

pub struct Error {
    pub kind: Kind,
    pub details: Cow<'static, str>,
}

impl Error {
    pub fn new<I>(kind: Kind, details: I) -> Error
        where I: Into<Cow<'static, str>>
    {
        Error {
            kind: kind,
            details: details.into(),
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.details.len() > 0 {
            write!(f, "Error <{:?}>: \"{}\" / \"{}\"", self.kind, self.description(), self.details)
        } else {
            write!(f, "Error <{:?}>: \"{}\"", self.kind, self.description())
        }
    }
}

impl StdError for Error {
    fn description(&self) -> &str {
        match self.kind {
            // Kind::InputUrlMissingHost => "missing host inside input URL",
            // Kind::InputUrlHostMustBeIpv4 => "provided host must be valid IPv4",
            Kind::Encoding(ref err) => err.description(),
            Kind::Io(ref err) => err.description(),
        }
    }

    fn cause(&self) -> Option<&StdError> {
        match self.kind {
            // Kind::InputUrlMissingHost => None,
            // Kind::InputUrlHostMustBeIpv4 => None,
            Kind::Encoding(ref err) => Some(err),
            Kind::Io(ref err) => Some(err),
        }
    }
}

from!(Utf8Error, Kind::Encoding);
from!(IoError, Kind::Io);
