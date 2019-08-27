use std::fmt;

// TODO: implement

/// ISO/IEC 13818-1
///
/// ISO 639 language descriptor
#[derive(Clone)]
pub struct Desc0x0A<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Desc0x0A<'buf> {
    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Desc0x0A<'buf> {
        Desc0x0A { buf }
    }
}

impl<'buf> fmt::Debug for Desc0x0A<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, ":0x0a")
    }
}
