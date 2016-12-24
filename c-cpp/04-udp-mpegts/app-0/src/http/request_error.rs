use std::io;
use std::fmt;
use std::error;
use std::str::Utf8Error;
use std::num::ParseIntError;
use std::result::Result as StdResult;


pub type RequestResult<T> = StdResult<T, RequestError>;


macro_rules! from {
    ($src:path, $dst:path) => {
        impl From<$src> for RequestError {
            fn from(err: $src) -> RequestError {
                $dst(err)
            }
        }
    }
}


#[derive(Debug)]
pub enum RequestError {
    IoReadUntil(io::Error),
    Utf8(Utf8Error),
    ParseInt(ParseIntError),
    RequestLineMissing,
    NoData,
}

impl fmt::Display for RequestError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            RequestError::IoReadUntil(ref err) => write!(f, "reader->read-until: {}", err),
            RequestError::Utf8(ref err) => write!(f, "decoding buffer into utf8: {}", err),
            RequestError::ParseInt(ref err) => write!(f, "parsing int: {}", err),
            RequestError::RequestLineMissing => write!(f, "missing request-line in TCP payload"),
            RequestError::NoData => write!(f, "missing data in TCP stream to parse"),
        }
    }
}

impl error::Error for RequestError {
    fn description(&self) -> &str {
        match *self {
            RequestError::IoReadUntil(ref err) => err.description(),
            RequestError::Utf8(ref err) => err.description(),
            RequestError::ParseInt(ref err) => err.description(),
            RequestError::RequestLineMissing => "missing request-line in TCP payload",
            RequestError::NoData => "missing data in TCP stream to parse",
        }
    }

    fn cause(&self) -> Option<&error::Error> {
        match *self {
            RequestError::IoReadUntil(ref err) => Some(err),
            RequestError::Utf8(ref err) => Some(err),
            RequestError::ParseInt(ref err) => Some(err),
            RequestError::RequestLineMissing => None,
            RequestError::NoData => None,
        }
    }
}

from!(io::Error, RequestError::IoReadUntil);
from!(Utf8Error, RequestError::Utf8);
from!(ParseIntError, RequestError::ParseInt);
