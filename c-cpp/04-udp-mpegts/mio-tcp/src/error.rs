use std::io;
use std::fmt;
use std::error::Error as StdError;


#[derive(Debug)]
pub enum Kind {
    Io(io::Error),
}

pub struct Error {
    pub kind: Kind,
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.kind {
            Kind::Io(ref err) => write!(f, "{:?}: {}", self.kind, err),
        }
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self)
    }
}

impl StdError for Error {

    fn description(&self) -> &str {
        match self.kind {
            Kind::Io(ref err) => err.description(),
        }
    }

    fn cause(&self) -> Option<&StdError> {
        match self.kind {
            Kind::Io(ref err) => Some(err),
        }
    }
}

impl From<io::Error> for Error {
    fn from(err: io::Error) -> Error {
        Error{kind: Kind::Io(err)}
    }
}
