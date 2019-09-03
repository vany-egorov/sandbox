use std::collections::HashMap;
use std::io::Cursor;

use crate::packet::Packet;
use crate::pid::PID;
use crate::result::Result;
use crate::subtable_id::{SubtableID, SubtableIDer};
use crate::table_id::TableID;
use crate::{EIT, PAT, SDT};

pub struct Buf(Cursor<Vec<u8>>);

impl Buf {
    fn new() -> Buf {
        Buf(Cursor::new(Vec::with_capacity(2048)))
    }
}

pub struct Table {}

impl Table {
    fn new() -> Table {
        Table {}
    }
}

pub struct Stream {
    buf: Buf,
}

impl Stream {
    fn new() -> Stream {
        Stream { buf: Buf::new() }
    }
}

#[derive(Default)]
pub struct Demuxer {
    tables: HashMap<SubtableID, Table>,
    streams: HashMap<PID, Stream>,
}

impl Demuxer {
    pub fn new() -> Demuxer {
        Default::default()
    }

    pub fn demux(&self, raw: &[u8]) -> Result<()> {
        let pkt = Packet::new(&raw)?;
        let pid = pkt.pid();

        if pid.is_null() {
            return Ok(());
        }

        match pid {
            PID::PAT => {
                let buf = pkt.buf_payload_section()?;
                let pat = PAT::try_new(buf)?;

                println!(">>>>>>> {:?}", pat.subtable_id());
            }
            PID::SDT => {
                let buf = pkt.buf_payload_section()?;
                let sdt = SDT::new(buf);

                println!(">>>>>>> {:?}", sdt.subtable_id());
            }
            PID::EIT => {
                let buf = pkt.buf_payload_section()?;
                let eit = EIT::new(buf);

                println!(">>>>>>> {:?}", eit.subtable_id());
            }
            PID::Other(..) => {}
            _ => return Ok(()),
        }

        Ok(())
    }
}
