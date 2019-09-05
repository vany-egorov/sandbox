use std::cell::RefCell;
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

    #[inline(always)]
    fn into_ref(self) -> SectionRef {
        Rc::new(RefCell::new(Box::new(self)))
    }
}

type SectionRef = Rc<RefCell<Box<Section>>>;

struct Sections(Vec<SectionRef>);

impl Sections {
    fn new() -> Sections {
        Default::default()
    }

    #[inline(always)]
    #[allow(dead_code)]
    fn get_mut(&mut self, number: u8) -> Option<&mut SectionRef> {
        self.0.iter_mut().find(|s| s.borrow().number == number)
    }

    #[inline(always)]
    fn push(&mut self, s: SectionRef) {
        self.0.push(s);
        self.0
            .sort_unstable_by(|a, b| a.borrow().number.cmp(&b.borrow().number));
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
    current: Option<SectionRef>,
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

                        let section_ref = match table.sections.get_mut(section_number) {
                            Some(section_ref) => {
                                {
                                    let mut section = (*section_ref).borrow_mut();

                                    {
                                        let raw = section.buf.0.get_ref().as_slice();

                                        match pid {
                                            PID::PAT => println!("{:?}", PAT::new(raw)),
                                            PID::SDT => println!("{:?}", SDT::new(raw)),
                                            PID::EIT => println!("{:?}", EIT::new(raw)),
                                            _ => {},
                                        };
                                    }

                                    section.buf.reset();
                                }

                                section_ref.clone()
                            },
                            None => {
                                let section_ref = Section::new(section_number).into_ref();

                                table.sections.push(section_ref.clone());

                                section_ref
                            },
                        };

                        tables.current = Some(section_ref);
                    }

                    if let Some(section_ref) = &tables.current {
                        let mut section = (*section_ref).borrow_mut();
                        section.buf.0.write_all(buf)?;
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
