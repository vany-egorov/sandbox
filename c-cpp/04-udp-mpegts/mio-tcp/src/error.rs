use std::io;
use std::fmt;
use std::borrow::Cow;
use std::convert::Into;
use std::str::Utf8Error;
use std::error::Error as StdError;

use http::RequestError as HTTPRequestError;


#[derive(Debug)]
pub enum Kind {
    NoListenerProvided,
    Capacity,
    NoConnectionAssociatedWithToken,
    Encoding(Utf8Error),
    Io(io::Error),
    HTTP(HTTPRequestError),
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
            Kind::NoListenerProvided              => "No listener provided",
            Kind::Capacity                        => "Out of capacity",
            Kind::NoConnectionAssociatedWithToken => "No connection associated with token",
            Kind::Io(ref err)                     => err.description(),
            Kind::Encoding(ref err)               => err.description(),
            Kind::HTTP(ref err)                   => err.description(),
        }
    }

    fn cause(&self) -> Option<&StdError> {
        match self.kind {
            Kind::NoListenerProvided              => None,
            Kind::Capacity                        => None,
            Kind::NoConnectionAssociatedWithToken => None,
            Kind::Io(ref err)                     => Some(err),
            Kind::Encoding(ref err)               => Some(err),
            Kind::HTTP(ref err)                   => Some(err),
        }
    }
}

impl From<Utf8Error> for Error {
    fn from(err: Utf8Error) -> Error {
        Error::new(Kind::Encoding(err), "")
    }
}

impl From<io::Error> for Error {
    fn from(err: io::Error) -> Error {
        Error::new(Kind::Io(err), "")
    }
}

impl From<HTTPRequestError> for Error {
    fn from(err: HTTPRequestError) -> Error {
        Error::new(Kind::HTTP(err), "")
    }
}

