extern crate encoding_rs;

use std::borrow::Cow;
use std::str;
use std::fmt;

use self::encoding_rs::ISO_8859_5;

/// Short event descriptor
///
/// ETSI EN 300 468 V1.15.1
///
/// The short event descriptor provides the name of the
/// event and a short description of the event in text form
#[derive(Clone)]
pub struct Desc0x4D<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Desc0x4D<'buf> {
    const HEADER_SZ: usize = 4;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Desc0x4D<'buf> {
        Desc0x4D { buf }
    }

    /// An 8-bit field specifying the length in bytes of the event name.
    #[inline(always)]
    fn event_name_length(&self) -> u8 {
        self.buf[3]
    }

    /// A string of "char" fields specifies the event name.
    /// Text information is coded using the character sets and
    /// methods described in annex A.
    #[inline(always)]
    fn event_name_raw(&self) -> &'buf [u8] {
        let lft = Self::HEADER_SZ;
        let rght = lft + (self.event_name_length() as usize);
        &self.buf[lft..rght]
    }

    /// An 8-bit field specifying the length in bytes of the event name.
    #[inline(always)]
    fn text_length(&self) -> u8 {
        self.buf[Self::HEADER_SZ+(self.event_name_length() as usize)]
    }

    /// A string of "char" fields specifies the event name.
    /// Text information is coded using the character sets and
    /// methods described in annex A.
    #[inline(always)]
    fn text_raw(&self) -> &'buf [u8] {
        let lft = Self::HEADER_SZ+(self.event_name_length() as usize)+1;
        let rght = lft + (self.text_length() as usize);
        &self.buf[lft..rght]
    }

    #[inline(always)]
    fn event_name(&self) -> Cow<'buf, str> {
        // TODO: error handling; multiple decoders;
        let (cow, _, _) = ISO_8859_5.decode(self.event_name_raw());
        cow
    }

    #[inline(always)]
    fn text(&self) -> Cow<'buf, str> {
        // TODO: error handling; multiple decoders;
        let (cow, _, _) = ISO_8859_5.decode(self.text_raw());
        cow
    }
}

impl<'buf> fmt::Debug for Desc0x4D<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, ":0x4d {}/{}", self.event_name(), self.text())
    }
}
