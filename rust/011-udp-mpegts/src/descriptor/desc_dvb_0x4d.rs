use std::fmt;
use crate::annex_a2::AnnexA2;

/// ETSI EN 300 468 V1.15.1
///
/// Short event descriptor
#[derive(Clone)]
pub struct DescDVB0x4D<'buf> {
    buf: &'buf [u8],
}

impl<'buf> DescDVB0x4D<'buf> {
    const HEADER_SZ: usize = 4;

    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> DescDVB0x4D<'buf> {
        DescDVB0x4D { buf }
    }

    #[inline(always)]
    fn buf_pos_event_name(&self) -> usize {
        Self::HEADER_SZ
    }

    #[inline(always)]
    fn buf_pos_text_length(&self) -> usize {
        self.buf_pos_event_name()+(self.event_name_length() as usize)
    }

    #[inline(always)]
    fn buf_pos_text(&self) -> usize {
        self.buf_pos_text_length() + 1
    }

    #[inline(always)]
    fn event_name_length(&self) -> u8 {
        self.buf[3]
    }

    #[inline(always)]
    fn event_name(&self) -> &'buf [u8] {
        &self.buf[self.buf_pos_event_name()..self.buf_pos_text_length()]
    }

    #[inline(always)]
    fn text_length(&self) -> u8 {
        self.buf[self.buf_pos_text_length()]
    }

    #[inline(always)]
    fn text(&self) -> &'buf [u8] {
        let lft = self.buf_pos_text();
        let rght = lft + (self.text_length() as usize);
        &self.buf[lft..rght]
    }
}

impl<'buf> fmt::Debug for DescDVB0x4D<'buf> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, ":dvb-0x4d (")?;

        write!(f, "event-name:")?;
        match AnnexA2::decode(self.event_name()) {
            Ok((v, _)) => write!(f, r#" "{}""#, v),
            Err(err) => write!(f, " (error: {:?})", err),
        }?;

        write!(f, " text:")?;
        match AnnexA2::decode(self.text()) {
            Ok((v, _)) => write!(f, r#" "{}""#, v),
            Err(err) => write!(f, " (error: {})", err),
        }?;

        write!(f, ")")
    }
}
