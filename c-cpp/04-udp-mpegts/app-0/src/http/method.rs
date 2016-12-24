#![allow(dead_code)]


use std::fmt;
use std::str::FromStr;


#[derive(Debug)]
pub enum ParseError {
    InvalidMethod
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

#[derive(Debug, Copy, Clone)]
pub enum Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT,
    PATCH,
    OPTIONS,
}

impl fmt::Display for Method {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl FromStr for Method {
    type Err = ParseError;

    fn from_str(s: &str) -> Result<Method, ParseError> {
        match s {
            "GET"     => Ok(Method::GET),
            "HEAD"    => Ok(Method::HEAD),
            "POST"    => Ok(Method::POST),
            "PUT"     => Ok(Method::PUT),
            "DELETE"  => Ok(Method::DELETE),
            "TRACE"   => Ok(Method::TRACE),
            "CONNECT" => Ok(Method::CONNECT),
            "PATCH"   => Ok(Method::PATCH),
            "OPTIONS" => Ok(Method::OPTIONS),
            _         => Err(ParseError::InvalidMethod),
        }
    }
}

impl<'a> From<&'a str> for Method {
    fn from(s: &'a str) -> Method {
        Method::from_str(s).unwrap()
    }
}

impl Default for Method {
    fn default() -> Method { Method::GET }
}
