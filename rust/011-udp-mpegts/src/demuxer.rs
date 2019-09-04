use std::collections::HashMap;
use std::io::{Cursor, Write};

use crate::packet::Packet;
use crate::pid::PID;
use crate::result::Result;
use crate::section::WithSyntaxSection;
use crate::subtable_id::{SubtableID, SubtableIDer};
use crate::{EIT, PAT, SDT};

#[derive(Default)]
struct Buf(Cursor<Vec<u8>>);

impl Buf {
    fn new() -> Buf {
        Buf(Cursor::new(Vec::with_capacity(2048)))
    }
}

struct Section {
    number: u8,
    buf: Buf,
}

impl Section {
    fn new(number: u8) -> Section {
        Section {
            number,
            buf: Buf::new(),
        }
    }
}

struct Sections(Vec<Section>);

impl Sections {
    fn new() -> Sections {
        Sections(Vec::with_capacity(1))
    }

    #[inline(always)]
    fn has(&self, number: u8) -> bool {
        self.0.iter().any(|s| s.number == number)
    }

    #[inline(always)]
    fn get_mut(&mut self, number: u8) -> Option<&mut Section> {
        self.0.iter_mut().find(|s| s.number == number)
    }

    #[inline(always)]
    fn push(&mut self, s: Section) {
        self.0.push(s);
        self.0.sort_unstable_by(|a, b| a.number.cmp(&b.number));
    }
}

struct Table {
    id: SubtableID,
    sections: Sections,
}

impl Table {
    fn new(id: SubtableID) -> Table {
        Table {
            id,
            sections: Sections::new(),
        }
    }
}

struct Tables {
    map: HashMap<SubtableID, Table>,
    /// current demuxing section
    current: Option<Section>,
}

impl Tables {
    fn new() -> Tables {
        Tables {
            map: HashMap::new(),
            current: None,
        }
    }
}

struct Stream {
    buf: Buf,
}

impl Stream {
    fn new() -> Stream {
        Stream { buf: Buf::new() }
    }
}

struct Streams {
    map: HashMap<PID, Stream>,
}

impl Streams {
    fn new() -> Streams {
        Streams {
            map: HashMap::new(),
        }
    }
}

/// TODO: use tree, redix tree here
/// TODO: add benches
pub struct Demuxer {
    pat: Tables,
    pmt: Tables,
    eit: Tables,
    sdt: Tables,

    streams: Streams,
}

impl Demuxer {
    pub fn new() -> Demuxer {
        Demuxer {
            pat: Tables::new(),
            pmt: Tables::new(),
            eit: Tables::new(),
            sdt: Tables::new(),

            streams: Streams::new(),
        }
    }

    /// mutably borrow a reference to the underlying tables
    /// by pid
    fn with_tables_mut<F>(&mut self, pid: PID, f: F) -> Result<()>
    where
        F: Fn(&mut Tables) -> Result<()>,
    {
        f(match pid {
            PID::PAT => &mut self.pat,
            PID::SDT => &mut self.sdt,
            PID::EIT => &mut self.eit,
            _ => unreachable!(),
        })
    }

    pub fn demux(&mut self, raw: &[u8]) -> Result<()> {
        let pkt = Packet::new(&raw)?;
        let pid = pkt.pid();

        if pid.is_null() {
            return Ok(());
        }

        match pid {
            PID::PAT | PID::SDT | PID::EIT /*| PID::NIT | PID::CAT | PID::BAT */  => {
                let buf = pkt.buf_payload_section()?;

                self.with_tables_mut(pid, |tables| {
                    if pkt.pusi() {
                        let (id, section_number) = match pid {
                            PID::PAT => {
                                let s = PAT::try_new(buf)?;
                                (s.subtable_id(), s.section_number())
                            }
                            PID::SDT => {
                                let s = SDT::try_new(buf)?;
                                (s.subtable_id(), s.section_number())
                            }
                            PID::EIT => {
                                let s = EIT::try_new(buf)?;
                                (s.subtable_id(), s.section_number())
                            }
                            _ => unreachable!()
                        };

                        let table = tables.map.entry(id).or_insert_with(|| Table::new(id));

                        if table.sections.has(section_number) {
                            // clear buffer
                        } else {
                            let mut section = Section::new(section_number);
                            section.buf.0.write_all(buf);

                            table.sections.push(section);
                        }
                    } else if let Some(ref mut section) = tables.current {

                    }

                    Ok(())
                })?;



                // let table = self.pat.entry(id).or_insert_with(|| {
                //     let mut table = Table::new();
                //     table.buf.0.write_all(buf).unwrap();

                //     println!(">>>>>>> {:?}", pat);

                //     table
                // });
            }
            PID::Other(..) => {
                // println!(">>>>>>> {:?}", pid);
            }
            _ => return Ok(()),
        }

        Ok(())
    }
}
