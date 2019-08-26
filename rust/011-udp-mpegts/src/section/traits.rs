use crate::table_id::TableID;
use crate::result::Result;
use std::marker::PhantomData;

pub trait Bufer<'buf> {
    /// borrow a reference to the underlying buffer
    #[inline(always)]
    fn buf(&self) -> &'buf [u8];
}

pub const HEADER_SZ: usize = 3;
#[allow(dead_code)]
pub const HEADER_MAX_SECTION_LENGTH: usize = 0x3FD; // 1021

pub(crate) trait WithHeader<'buf>: Bufer<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        self.buf()
    }

    /// table_id
    /// - The table_id identifies to which table the section belongs.
    /// - Some table_ids have been defined by ISO and others by ETSI.
    ///   Other values of the table_id can be allocated by the user
    ///   for private purposes.
    #[inline(always)]
    fn table_id(&self) -> TableID {
        TableID::from(self.b()[0])
    }

    /// This is a 12-bit field, the first two bits of which shall be '00'.
    /// The remaining 10 bits specify the number of bytes of the section,
    /// starting immediately following the section_length field,
    /// and including the CRC. The value in this
    /// field shall not exceed 1021 (0x3FD)
    #[inline(always)]
    fn section_length(&self) -> u16 {
        (((self.b()[1] & 0b0000_1111) as u16) << 8) | self.b()[2] as u16
    }
}

pub const SYNTAX_SECTION_SZ: usize = 5;

pub(crate) trait WithSyntaxSection<'buf>: Bufer<'buf> {
    /// buffer seeked
    #[inline(always)]
    fn b(&self) -> &'buf [u8] {
        &self.buf()[HEADER_SZ..]
    }

    /// This is a 16-bit field which serves as a label to identify
    /// this Transport Stream from any other multiplex within a network.
    /// Its value is defined by the user.
    #[inline(always)]
    #[allow(dead_code)]
    fn transport_stream_id(&self) -> u16 {
        ((self.b()[0] as u16) << 8) | (self.b()[1] as u16)
    }

    /// This 5-bit field is the version number of the whole
    /// Program Association Table. The version number
    /// shall be incremented by 1 modulo 32 whenever the definition
    /// of the Program Association Table changes. When the
    /// current_next_indicator is set to '1', then the version_number
    /// shall be that of the currently applicable Program Association
    /// Table. When the current_next_indicator is set to '0',
    /// then the version_number shall be that of the next applicable Program
    /// Association Table.
    #[inline(always)]
    #[allow(dead_code)]
    fn version_number(&self) -> u8 {
        (self.b()[2] & 0b0011_1110) >> 1
    }

    /// A 1-bit indicator, which when set to '1' indicates
    /// that the Program Association Table sent is currently applicable.
    /// When the bit is set to '0', it indicates that the table
    // sent is not yet applicable and shall be the next table to become valid.
    #[inline(always)]
    fn current_next_indicator(&self) -> bool {
        (self.b()[2] & 0b0000_0001) != 0
    }

    /// This 8-bit field gives the number of this section.
    /// The section_number of the first section in the Program Association
    /// Table shall be 0x00. It shall be incremented by 1
    /// with each additional section in the Program Association Table.
    #[inline(always)]
    fn section_number(&self) -> u8 {
        self.b()[3]
    }

    /// This 8-bit field specifies the number of the last section
    /// (that is, the section with the highest section_number)
    /// of the complete Program Association Table.
    #[inline(always)]
    fn last_section_number(&self) -> u8 {
        self.b()[4]
    }
}

pub trait Szer {
    fn sz(&self) -> usize;
}

pub trait TryNewer<'buf> {
    fn try_new(buf: &'buf [u8]) -> Result<Self>
        where Self: Sized;
}

pub struct Cursor<'buf, T> {
    buf: &'buf [u8],
    phantom: PhantomData<T>,
}

impl<'buf, T> Cursor<'buf, T> {
    #[inline(always)]
    pub fn new(buf: &'buf [u8]) -> Cursor<'buf, T> {
        Cursor { buf, phantom: PhantomData }
    }

    #[inline(always)]
    fn buf_drain(&mut self) {
        self.buf = &self.buf[self.buf.len()..];
    }
}

impl<'buf, T> Iterator for Cursor<'buf, T>
    where T: TryNewer<'buf> + Szer
{
    type Item = Result<T>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.buf.len() == 0 {
            return None
        }

        let row = match T::try_new(self.buf) {
            Ok(row) => row,
            Err(e) => {
                self.buf_drain();
                return Some(Err(e));
            },
        };

        // seek buf
        if self.buf.len() > row.sz() {
            self.buf = &self.buf[row.sz()..];
        } else {
            self.buf_drain();
        }

        Some(Ok(row))
    }
}

pub const CRC32_SZ: usize = 4;

pub(crate) trait WithCRC32<'buf>: Bufer<'buf> {}
