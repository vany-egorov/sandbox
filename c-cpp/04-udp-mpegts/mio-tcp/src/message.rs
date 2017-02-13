use result::Result;

// message sent through BUS
pub struct Message {
    pub kind: Kind,
    pub body: Body,
}

pub enum Kind {
    // broadcast data across all connected
    // websocket clients
    WsBroadcast,

    // shutdown server
    Shutdown,
}

pub enum Body {
    Text(String),
    Bin(Vec<u8>),
}

impl Body {
    pub fn into_bin(self) -> Vec<u8> {
        match self {
            Body::Text(s) => s.into_bytes(),
            Body::Bin(d) => d,
        }
    }

    pub fn into_string(self) -> Result<String> {
        match self {
            Body::Text(s) => Ok(s),
            Body::Bin(d) => Ok(try!(
                String::from_utf8(d).map_err(|err| err.utf8_error()))),
        }
    }
}

