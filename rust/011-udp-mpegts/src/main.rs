mod error;

#[macro_use]
extern crate nom;
extern crate clap;
extern crate url;

use clap::{App, Arg};
use nom::IResult;
use std::collections::VecDeque;
use std::net::{Ipv4Addr, UdpSocket};
use std::process;
use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::Duration;
use url::{Host /*, ParseError*/, Url};

use error::{Error, Kind as ErrorKind, Result};

const TS_SYNC_BYTE: u8 = 0x47;
const TS_PKT_SZ: usize = 188;

const TS_PID_PAT: u16 = 0x0000;
const TS_PID_CAT: u16 = 0x0001;
const TS_PID_TSDT: u16 = 0x0002;
const TS_PID_CIT: u16 = 0x0003;
const TS_PID_SDT: u16 = 0x0011;
const TS_PID_NULL: u16 = 0x1FFF;

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSHeader {
    // transcport-error-indicator
    // :1
    tei: u8,

    // payload-unit-start-indicator
    // Set when a PES, PSI, or DVB-MIP
    // packet begins immediately following the header.
    // :1
    pusi: u8,

    // transport-priority
    // :1
    tp: u8,

    // :13
    pid: u16,

    // transport-scrambling-control
    // :2
    tsc: u8,

    // adaptation-field-control
    // :1
    afc: u8,

    // :1
    contains_payload: u8,

    // continuity-counter
    // :4
    cc: u8,
}

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSPCR {
    // :33
    base: u64,

    // :9
    ext: u16,
}

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSAdaptation {
    // adaptation-field-length
    // :8
    afl: u8,

    // discontinuity-indicator
    // :1
    di: u8,

    // random-access-indicator
    // :1
    rai: u8,

    // elementary-stream-priority-indicator
    // :1
    espi: u8,

    // PCR-flag
    // :1
    pcr_flag: u8,

    // OPCR-flag
    // :1
    opcr_flag: u8,

    // splicing-point-flag
    // :1
    spf: u8,

    // transport-private-data-flag
    // :1
    tpdf: u8,

    // adaptation-field-extension-flag
    // :1
    afef: u8,

    pcr: Option<TSPCR>,
}

// Program Specific Information
#[allow(dead_code)]
#[derive(Debug)]
pub struct TSPSI {
    table_id: u8,

    // section-syntax-indicator
    // :1
    ssi: u8,

    // :1
    private_bit: u8,

    // :2
    reserved_bits: u8,

    // section-length-unused-bits
    // :2
    slub: u8,

    // section-length
    //
    // This is a 12-bit field, the first two bits of which shall be "00".
    // It specifies the number of bytes of the
    // section, starting immediately following
    // the section_length field and including the CRC.
    // The section_length shall not
    // exceed 1 021 so that the entire section has a maximum length
    // of 1 024 bytes.
    //
    // :10
    section_length: u16,

    tsi: u16,

    // :2

    // version-number
    // :5
    vn: u8,

    // curent-next-indicator
    // :1
    cni: u8,

    // section-number
    // :8
    sn: u8,

    // last-section-number
    // :8
    lsn: u8,

    // :32
    crc32: u32,
}

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSPSIPAT {
    psi: TSPSI,

    // Relates to the Table ID extension in the associated PMT.
    // A value of 0 is reserved for a NIT packet identifier.
    program_number: u16,

    // The packet identifier that
    // contains the associated PMT
    program_map_pid: u16,
}

#[cfg_attr(rustfmt, rustfmt_skip)]
pub fn parse_ts_header(input: &[u8]) -> IResult<&[u8], TSHeader> {
    do_parse!(input,
        b1: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 5)
        ))
        >> b2: bits!(take_bits!(u8, 8))
        >> b3: bits!(tuple!(
            take_bits!(u8, 2),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 4)
        ))

        >> (TSHeader {
            tei: b1.0,
            pusi: b1.1,
            tp: b1.2,
            pid: ((b1.3 as u16) << 8) | b2 as u16,
            tsc: b3.0,
            afc: b3.1,
            contains_payload: b3.2,
            cc: b3.3,
        })
    )
}

#[cfg_attr(rustfmt, rustfmt_skip)]
pub fn parse_ts_adaptation(input: &[u8]) -> IResult<&[u8], TSAdaptation> {;
    let (input, mut ts_adaptation) = try!(do_parse!(input,
           b1: bits!(take_bits!(u8, 8))
        >> b2: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1)
        ))

        >> (TSAdaptation {
            afl: b1,

            di: b2.0,
            rai: b2.1,
            espi: b2.2,
            pcr_flag: b2.3,
            opcr_flag: b2.4,
            spf: b2.5,
            tpdf: b2.6,
            afef: b2.7,

            pcr: None,
        })
    ));

    if ts_adaptation.pcr_flag == 0 {
        return Ok((input, ts_adaptation))
    }

    let (input, ts_pcr) = try!(do_parse!(input,
           b1: bits!(take_bits!(u8, 8))
        >> b2: bits!(take_bits!(u8, 8))
        >> b3: bits!(take_bits!(u8, 8))
        >> b4: bits!(take_bits!(u8, 8))
        >> b5: bits!(take_bits!(u8, 1))

        >> _b5_6: bits!(take_bits!(u8, 6))

        >> b6: bits!(take_bits!(u8, 1))
        >> b7: bits!(take_bits!(u8, 8))

        >> (TSPCR {
            base: (
                ((b1 as u64) << 25) |
                ((b2 as u64) << 17) |
                ((b3 as u64) << 9) |
                ((b4 as u64) << 1) |
                 (b5 as u64)
            ),
            ext: (
                ((b6 as u16) << 8) |
                 (b7 as u16)
            ),
        })
    ));

    ts_adaptation.pcr = Some(ts_pcr);

    Ok((input, ts_adaptation))
}

#[cfg_attr(rustfmt, rustfmt_skip)]
pub fn parse_ts_psi(input: &[u8]) -> IResult<&[u8], TSPSI> {
    do_parse!(input,
           b1: bits!(take_bits!(u8, 8))
        >> b2: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 2),
            take_bits!(u8, 2),
            take_bits!(u8, 2)
        ))
        >> b3: bits!(take_bits!(u8, 8))
        >> b4: bits!(take_bits!(u8, 8))
        >> b5: bits!(take_bits!(u8, 8))
        >> b6: bits!(tuple!(
            take_bits!(u8, 2),
            take_bits!(u8, 5),
            take_bits!(u8, 1)
        ))
        >> b7: bits!(take_bits!(u8, 8))
        >> b8: bits!(take_bits!(u8, 8))

        >> (TSPSI {
            table_id: b1,

            ssi: b2.0,
            private_bit: b2.1,
            reserved_bits: b2.2,
            slub: b2.3,
            section_length: ((b2.4 as u16) << 8) | b3 as u16,

            tsi: ((b4 as u16) << 8) | b5 as u16,

            vn: b6.1,
            cni: b6.2,

            sn: b7,
            lsn: b8,

            // 3 bytes of PSI header
            // [0, 1, 2, ..., -3, -2, -1, -0]
            // e.g. section-length = 5;
            //      [0, 1, 2, 3  , 4      , 5      , 6      , 7]
            //      [header, ...
            //          ..., data, crc32-0, crc32-1, crc32-2, crc32-3]
            //      => section starts at data[3] where 3 is 2 + 1
            //      => crc32-i = 7 = 2 + section-length
            crc32: 0
        })
    )
}

#[cfg_attr(rustfmt, rustfmt_skip)]
pub fn parse_ts_psi_pat(input: &[u8]) -> IResult<&[u8], TSPSIPAT> {
    let (input, ts_psi_pat) = try!(parse_ts_psi(input));

    do_parse!(input,
           b1: bits!(take_bits!(u8, 8))
        >> b2: bits!(take_bits!(u8, 8))
        >> b3: bits!(take_bits!(u8, 8))
        >> b4: bits!(take_bits!(u8, 8))

        >> (TSPSIPAT {
            psi: ts_psi_pat,

            program_number: ((b1 as u16) << 8) | b2 as u16,
            program_map_pid: ((b3 as u16) << 8) | b4 as u16,
        })
    )
}

#[cfg_attr(rustfmt, rustfmt_skip)]
named!(parse_ts_single<&[u8], TSHeader>, do_parse!(
    tag!(&[TS_SYNC_BYTE])      >> // 1
    ts_header: parse_ts_header >> // 3

    (ts_header)));

#[cfg_attr(rustfmt, rustfmt_skip)]
named!(
    parse_ts_multi<&[u8], Vec<TSHeader>>,
    many1!(parse_ts_single)
);

struct DemuxerTS {}

impl DemuxerTS {
    pub fn new() -> DemuxerTS {
        DemuxerTS {}
    }
}

trait Input {
    fn open(&mut self) -> Result<()>;
    fn read(&mut self) -> Result<()>;
    fn close(&mut self) -> Result<()>;
}

trait Filter {
    fn consume_strm(&self);
    fn consume_trk(&self);
    fn consume_pkt_raw(&self);
    fn consume_pkt(&self);
    fn consume_frm(&self);

    fn produce_strm(&self);
    fn produce_trk(&self);
    fn produce_pkt_raw(&self);
    fn produce_pkt(&self);
    fn produce_frm(&self);
}

struct InputUDP {
    url: Url,

    // circullar-buffer / fifo
    buf: Arc<(Mutex<VecDeque<[u8; TS_PKT_SZ]>>, Condvar)>,

    socket: Option<UdpSocket>,
}

impl InputUDP {
    pub fn new(url: Url, buf_cap: usize) -> InputUDP {
        InputUDP {
            url: url,
            buf: Arc::new((Mutex::new(VecDeque::with_capacity(buf_cap)), Condvar::new())),

            socket: None,
        }
    }
}

impl Input for InputUDP {
    fn open(&mut self) -> Result<()> {
        let input_host = try!(self
            .url
            .host()
            .ok_or(Error::new(ErrorKind::InputUrlMissingHost, "")));

        let input_port = self.url.port().unwrap_or(5500);

        let input_host_domain = try!(match input_host {
            Host::Domain(v) => Ok(v),
            _ => Err(Error::new(ErrorKind::InputUrlHostMustBeDomain, "")),
        });

        let iface = Ipv4Addr::new(0, 0, 0, 0);
        // let socket = try!(UdpSocket::bind((input_host_domain, input_port)));;

        // let iface = Ipv4Addr::new(127, 0, 0, 1);
        println!(
            "[<] {:?}: {:?} @ {:?}",
            input_host_domain, input_port, iface
        );

        let input_host_ip_v4: Ipv4Addr = input_host_domain.parse().unwrap();

        let socket = try!(UdpSocket::bind((input_host_domain, input_port)));

        if let Err(e) = socket.join_multicast_v4(&input_host_ip_v4, &iface) {
            eprintln!("error join-multiocast-v4: {}", e);
        }

        let pair = self.buf.clone();
        thread::spawn(move || {
            let mut ts_pkt_raw: [u8; TS_PKT_SZ] = [0; TS_PKT_SZ];

            loop {
                // MTU (maximum transmission unit) == 1500 for Ethertnet
                // 7*TS_PKT_SZ = 7*188 = 1316 < 1500 => OK
                let mut pkts_raw = [0; 7 * TS_PKT_SZ];
                let (_, _) = socket.recv_from(&mut pkts_raw).unwrap();

                let &(ref lock, ref cvar) = &*pair;
                let mut buf = lock.lock().unwrap();

                for pkt_index in 0..7 * TS_PKT_SZ / TS_PKT_SZ {
                    let ts_pkt_raw_src =
                        &pkts_raw[pkt_index * TS_PKT_SZ..(pkt_index + 1) * TS_PKT_SZ];

                    // println!("#{:?} -> [{:?} .. {:?}]; src-len: {:?}, dst-len: {:?}",
                    //     pkt_index, pkt_index*TS_PKT_SZ, (pkt_index+1)*TS_PKT_SZ,
                    //     ts_pkt_raw_src.len(), ts_pkt_raw.len(),
                    // );

                    ts_pkt_raw.copy_from_slice(ts_pkt_raw_src);
                    buf.push_back(ts_pkt_raw);
                }

                cvar.notify_all();
            }
        });

        Ok(())
    }

    fn read(&mut self) -> Result<()> {
        let pair = self.buf.clone();
        let &(ref lock, ref cvar) = &*pair;
        let mut buf = try!(lock
            .lock()
            .ok()
            .ok_or(Error::new(ErrorKind::SyncPoison, "udp read lock error")));

        buf = try!(cvar.wait(buf).ok().ok_or(Error::new(
            ErrorKind::SyncPoison,
            "udp read cwar wait erorr"
        )));

        while !buf.is_empty() {
            // TODO: move to function;
            let ts_pkt_raw = buf.pop_front().unwrap();

            let (ts_pkt_raw, ts_header) = try!(parse_ts_single(&ts_pkt_raw));
            if ts_header.afc == 1 {
                println!(
                    "pid: 0x{:04X}/{}, cc: {}",
                    ts_header.pid, ts_header.pid, ts_header.cc
                );

                let (_, ts_adaptation) = try!(parse_ts_adaptation(&ts_pkt_raw));
                println!(
                    "adaptation (:pcr? {:?} :adaptation-field-length {:?})",
                    ts_adaptation.pcr_flag, ts_adaptation.afl
                );

                if let Some(ref pcr) = ts_adaptation.pcr {
                    println!(
                        "pcr: {} / {} / 0:0:0:XXX ({})",
                        pcr.base,
                        pcr.ext,
                        pcr.base * 300,
                    );
                }

            }

            if ts_header.pid == TS_PID_PAT {
                let (_, ts_psi_pat) = try!(parse_ts_psi_pat(&ts_pkt_raw));
                println!("pat ({:?})", ts_psi_pat);
            }

            // match res {
            //     Ok((_, (_, ts_header, ts_pkt_tail))) => {
            //         if ts_header.afc == 1 {
            //             println!("pid: 0x{:04X}/{}, cc: {}", ts_header.pid, ts_header.pid, ts_header.cc);

            //             let res = parse_ts_adaptation(&ts_pkt_tail);
            //             match res {
            //                 Ok((ts_pkt_tail, ts_adaptation)) => {
            //                     println!("adaptation (:pcr? {:?} :adaptation-field-length {:?})",
            //                         ts_adaptation.pcr_flag,
            //                         ts_adaptation.afl);
            //                 },
            //                 _ => {}
            //             }
            //         } else {

            //         }
            //     },
            //     _  => {
            //         println!("error or incomplete cap: {:?}, len: {:?}, data: 0x{:02X?}{:02X?}{:02X?}",
            //             buf.capacity(), buf.len(), ts_pkt_raw[0], ts_pkt_raw[1], ts_pkt_raw[2]);
            //     }
            // }
        }

        Ok(())
    }

    fn close(&mut self) -> Result<()> {
        println!("<<< UDP close");

        Ok(())
    }
}

struct InputFile {
    url: Url,
}

impl InputFile {
    pub fn new(url: Url) -> InputFile {
        InputFile { url: url }
    }
}

impl Input for InputFile {
    fn open(&mut self) -> Result<()> {
        println!("<<< File open");

        Ok(())
    }

    fn read(&mut self) -> Result<()> {
        thread::sleep(Duration::from_secs(1000));

        Ok(())
    }

    fn close(&mut self) -> Result<()> {
        println!("<<< File close");

        Ok(())
    }
}

struct Wrkr<I> {
    input: Arc<Mutex<I>>,
}

impl<I> Wrkr<I>
where
    I: Input + std::marker::Send + 'static,
{
    pub fn new(input: I) -> Wrkr<I> {
        Wrkr {
            input: Arc::new(Mutex::new(input)),
        }
    }

    pub fn run(&self) -> Result<()> {
        let input = self.input.clone();
        {
            try!(input.lock().unwrap().open());
        }

        thread::spawn(move || loop {
            input.lock().unwrap().read();
        });

        Ok(())
    }
}

// Input #0, mpegts, from 'udp://239.255.1.1:5500':
//   Duration: N/A, start: 8174.927322, bitrate: N/A
//   Program 20105
//     Metadata:
//       service_name    : ?HD
//       service_provider: ~~~
//     Stream #0:0[0xcd]: Video: h264 (High) ([27][0][0][0] / 0x001B), yuv420p(tv, bt709), 1920x1080 [SAR 1:1 DAR 16:9], 25 fps, 50 tbr, 90k tbn, 50 tbc
//     Stream #0:1[0x131](rus): Audio: mp2 ([4][0][0][0] / 0x0004), 48000 Hz, stereo, s16p, 192 kb/s
//     Stream #0:2[0x195](rus): Audio: ac3 (AC-3 / 0x332D4341), 48000 Hz, 5.1(side), fltp, 384 kb/s
//     Stream #0:3[0x1f9](rus,rus): Subtitle: dvb_teletext ([6][0][0][0] / 0x0006)
fn main() {
    // let args: Vec<String> = env::args().collect();
    // println!("{:?}", args);
    let matches = App::new("V/A tool")
        .version("0.0.1")
        .author("Ivan Egorov <vany.egorov@gmail.com>")
        .about("Video/audio swiss knife")
        .arg(
            Arg::with_name("input")
                // .index(1)
                .short("i")
                .long("input")
                .help("Sets the input file to use")
                .required(true)
                .takes_value(true),
        )
        .get_matches();

    let input_raw = matches.value_of("input").unwrap();
    let input_url = match Url::parse(input_raw) {
        Ok(v) => v,
        Err(err) => {
            eprintln!("error parse input url: {:?}\n", err);
            process::exit(1);
        }
    };

    let input_url_1 = input_url.clone();
    let input_url_2 = input_url.clone();

    // <input builder based on URL>
    let input_udp = InputUDP::new(input_url_1, 5000 * 7);
    let input_file = InputFile::new(input_url_2);
    // </input builder based on URL>

    let wrkr1 = Wrkr::new(input_udp);
    let wrkr2 = Wrkr::new(input_file);

    if let Err(err) = wrkr1.run() {
        eprintln!("error start worker №1: {:?}\n", err);
        process::exit(1);
    }

    if let Err(err) = wrkr2.run() {
        eprintln!("error start worker №2: {:?}\n", err);
        process::exit(1);
    }

    loop {
        thread::sleep(Duration::from_secs(60));
    }
}
