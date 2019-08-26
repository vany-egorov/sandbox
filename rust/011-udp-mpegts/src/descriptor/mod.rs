mod tag;
mod desc_0x4d;

use std::fmt;
use std::str;

use crate::result::Result;
use crate::error::{Error, Kind as ErrorKind};
use crate::section::{Szer, TryNewer};

use self::tag::{Tag, TagDVB};
use self::desc_0x4d::Desc0x4D;

#[derive(Clone)]
pub struct Descriptor<'buf> {
    buf: &'buf [u8],
}

impl<'buf> Descriptor<'buf> {
    const HEADER_SZ: usize = 2;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Descriptor<'buf> {
        Descriptor { buf }
    }

    #[inline(always)]
    pub fn validate(&self) -> Result<()> {
        if self.buf.len() < Self::HEADER_SZ {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), Self::HEADER_SZ)))
        } else if self.buf.len() < self.sz() {
            Err(Error::new(ErrorKind::Buf(self.buf.len(), self.sz())))
        } else {
            Ok(())
        }
    }

    /// the descriptor tag is an 8-bit field which identifies each descriptor.
    /// Those values with MPEG-2
    /// normative meaning are described in ISO/IEC 13818-1 [18].
    /// (descriptor_tag :8 uimsbf)
    #[inline(always)]
    fn tag(&self) -> Tag { Tag::from(self.buf[0]) }

    /// the descriptor length is an 8-bit field specifying the total
    /// number of bytes of the data portion of the descriptor
    /// following the byte defining the value of this field.
    /// (descriptor_length :8 uimsbf)
    #[inline(always)]
    fn len(&self) -> u8 { self.buf[1] }

    /// seek
    #[inline(always)]
    fn buf_data(&self) -> &'buf [u8] {
        &self.buf[Self::HEADER_SZ..]
    }

    #[inline(always)]
    fn data_as_unicode(&'buf self) -> &'buf str {
        str::from_utf8(&self.buf_data())
            .unwrap_or("---")
    }
}

impl<'buf> Szer for Descriptor<'buf> {
    #[inline(always)]
    fn sz(&self) -> usize {
        Self::HEADER_SZ + (self.len() as usize)
    }
}

impl<'buf> TryNewer<'buf> for Descriptor<'buf> {
    #[inline(always)]
    fn try_new(buf: &'buf [u8]) -> Result<Descriptor<'buf>> {
        let d = Descriptor::new(buf);
        d.validate()?;
        Ok(d)
    }
}

impl<'buf> fmt::Debug for Descriptor<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "(:desc (:tag {:?} :length {}",
            self.tag(), self.len())?;
        write!(f, "\n          ")?;

        match self.tag() {
            Tag::DVB(TagDVB::ShortEvent) => {
                Desc0x4D::new(self.buf_data()).fmt(f)?;
            },
            _ =>  { write!(f, ":data {}", self.data_as_unicode())?; }
        }

        write!(f, "))")
    }
}
