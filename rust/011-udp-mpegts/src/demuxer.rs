use std::borrow::BorrowMut;
use std::collections::HashMap;
use std::io::{Cursor, Write};
use std::rc::Rc;

use crate::packet::Packet;
use crate::pid::PID;
use crate::result::Result;
use crate::section::WithSyntaxSection;
use crate::subtable_id::{SubtableID, SubtableIDer};
use crate::{EIT, PAT, SDT};

struct Buf(Cursor<Vec<u8>>);

impl Buf {
    #[inline(always)]
    fn reset(&mut self) {
        self.0.set_position(0);
        self.0.get_mut().clear();
    }
}

impl Default for Buf {
    fn default() -> Self {
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
            buf: Default::default(),
        }
    }
}

struct Sections(Vec<Rc<Box<Section>>>);

impl Sections {
    fn new() -> Sections {
        Default::default()
    }

    #[inline(always)]
    fn has(&self, number: u8) -> bool {
        self.0.iter().any(|s| s.number == number)
    }

    #[inline(always)]
    #[allow(dead_code)]
    fn get_mut(&mut self, number: u8) -> Option<&mut Rc<Box<Section>>> {
        self.0.iter_mut().find(|s| s.number == number)
    }

    #[inline(always)]
    fn push(&mut self, s: Rc<Box<Section>>) {
        self.0.push(s);
        self.0.sort_unstable_by(|a, b| a.number.cmp(&b.number));
    }
}

impl Default for Sections {
    fn default() -> Self {
        Sections(Vec::with_capacity(1))
    }
}

struct Table {
    #[allow(dead_code)]
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
    current: Option<Rc<Box<Section>>>,
}

impl Tables {
    fn new() -> Tables {
        Tables {
            map: HashMap::new(),
            current: None,
        }
    }
}

#[derive(Default)]
struct Stream {
    #[allow(dead_code)]
    buf: Buf,
}

impl Stream {}

#[derive(Default)]
struct Streams {
    #[allow(dead_code)]
    map: HashMap<PID, Stream>,
}

impl Streams {}

/// TODO: use tree, redix tree here
/// TODO: add benches
pub struct Demuxer {
    pat: Tables,
    #[allow(dead_code)]
    pmt: Tables,
    eit: Tables,
    sdt: Tables,

    #[allow(dead_code)]
    streams: Streams,
}

impl Default for Demuxer {
    fn default() -> Self {
        Demuxer {
            pat: Tables::new(),
            pmt: Tables::new(),
            eit: Tables::new(),
            sdt: Tables::new(),

            streams: Default::default(),
        }
    }
}

impl Demuxer {
    pub fn new() -> Demuxer {
        Default::default()
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

                        match table.sections.get_mut(section_number) {
                            Some(ref mut section) => { section.buf.reset(); },
                            None => {
                                let section = Rc::new(Box::new(Section::new(section_number)));
                                section.buf.0.write_all(buf)?;

                                table.sections.push(section);
                                tables.current = Some(section);
                            },
                        }
                    } else if let Some(ref mut section) = tables.current {
                        section.borrow_mut().buf.0.write_all(buf)?;
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
