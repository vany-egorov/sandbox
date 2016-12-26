use std::io;
use std::fmt;
use std::borrow::Cow;
use std::convert::Into;
use std::error::Error as StdError;


#[derive(Debug)]
pub enum Kind {
    NoListenerProvided,
    Capacity,
    Io(io::Error),
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
            Kind::NoListenerProvided => "No listener provided",
            Kind::Capacity           => "Out of capacity",
            Kind::Io(ref err)        => err.description(),
        }
    }

    fn cause(&self) -> Option<&StdError> {
        match self.kind {
            Kind::NoListenerProvided => None,
            Kind::Capacity           => None,
            Kind::Io(ref err)        => Some(err),
        }
    }
}

impl From<io::Error> for Error {
    fn from(err: io::Error) -> Error {
        Error::new(Kind::Io(err), "")
    }
}
