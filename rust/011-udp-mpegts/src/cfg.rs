#[derive(Debug)]
pub enum Kind {
    NoListenerProvided,
    Capacity,
    NoConnectionAssociatedWithToken,
    Encoding(Utf8Error),
    Io(io::Error),
    HTTP(HTTPRequestError),
}
