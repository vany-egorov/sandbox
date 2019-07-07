mod error;

#[macro_use]
extern crate nom;
extern crate url;
extern crate clap;

use std::process;
use std::time::Duration;
use std::collections::VecDeque;
use std::net::{UdpSocket, Ipv4Addr};
use std::thread;
use std::sync::{Arc, Mutex, Condvar};
use nom::IResult;
use clap::{Arg, App};
use url::{Url, Host/*, ParseError*/};

use error::{Result, Error, Kind as ErrorKind};


const TS_SYNC_BYTE: u8 = 0x47;
const TS_PKT_SZ: usize = 188;


#[allow(dead_code)]
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
    cc: u8
}

#[allow(dead_code)]
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

    // PCR_flag
    // :1
    pcr_flag: u8,

    // OPCR_flag
    // :1
    opcr_flag: u8,

    // splicing_point_flag
    // :1
    spf: u8,

    // transport_private_data_flag
    // :1
    tpdf: u8,

    // adaptation_field_extension_flag
    // :1
    afef: u8
}


pub fn parse_ts_sync_byte(input:&[u8]) -> IResult<&[u8], u8> {
  do_parse!(input,
    sync_byte: tag!(&[TS_SYNC_BYTE]) >>

    (*sync_byte.first().unwrap())
  )
}

pub fn parse_ts_header(input:&[u8]) -> IResult<&[u8], TSHeader> {
    do_parse!(input,
        b1: bits!(tuple!(
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 5)
        )) >>
        b2: bits!(take_bits!(u8, 8)) >>
        b3: bits!(tuple!(
            take_bits!(u8, 2),
            take_bits!(u8, 1),
            take_bits!(u8, 1),
            take_bits!(u8, 4)
        )) >>

        (TSHeader{
            tei: b1.0,
            pusi: b1.1,
            tp: b1.2,
            pid: ((b1.3 as u16) << 8) | b2 as u16,
            tsc: b3.0,
            afc: b3.1,
            contains_payload: b3.2,
            cc: b3.3
        })
    )
}

pub fn parse_ts_adaptation(input:&[u8]) -> /*IResult<&[u8], TSAdaptation>*/Result<()> {
    // TODO: implement
    Ok(())
}

named!(
    parse_ts_single<&[u8], (u8, TSHeader, &[u8])>,
    tuple!(
        parse_ts_sync_byte, // 1
        parse_ts_header,    // 3
        take!(184)          // 188 - 1 - 3 = 184
    )
);

named!(
    parse_ts_multi<&[u8], Vec<(u8, TSHeader, &[u8])>>,
    many1!(parse_ts_single)
);

struct DemuxerTS {
}

impl DemuxerTS {
    pub fn new() -> DemuxerTS {
        DemuxerTS{}
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
        InputUDP{
            url: url,
            buf: Arc::new((Mutex::new(VecDeque::with_capacity(buf_cap)), Condvar::new())),

            socket: None,
        }
    }
}

impl Input for InputUDP {
    fn open(&mut self) -> Result<()> {
        let input_host = try!(self.url.host()
            .ok_or(Error::new(ErrorKind::InputUrlMissingHost, "")));

        let input_port = self.url.port().unwrap_or(5500);

        let input_host_domain = try!(match input_host {
            Host::Domain(v) => Ok(v),
            _ => Err(Error::new(ErrorKind::InputUrlHostMustBeDomain, "")),
        });

        let iface = Ipv4Addr::new(0, 0, 0, 0);
        // let socket = try!(UdpSocket::bind((input_host_domain, input_port)));;

        // let iface = Ipv4Addr::new(127, 0, 0, 1);
        println!("[<] {:?}: {:?} @ {:?}", input_host_domain, input_port, iface);

        let input_host_ip_v4: Ipv4Addr = input_host_domain.parse().unwrap();

        let socket = UdpSocket::bind((input_host_domain, input_port))?;
        println!("OK bind");

        if socket.join_multicast_v4(&input_host_ip_v4, &iface).is_err() {
            eprintln!("ERR join");
        } else {
            println!("OK join");
        }

        let pair = self.buf.clone();
        thread::spawn(move || {
            let mut ts_pkt_raw: [u8; TS_PKT_SZ] = [0; TS_PKT_SZ];

            loop {
                // MTU (maximum transmission unit) == 1500 for Ethertnet
                // 7*TS_PKT_SZ = 7*188 = 1316 < 1500 => OK
                let mut pkts_raw = [0; 7*TS_PKT_SZ];
                let (_, _) = socket.recv_from(&mut pkts_raw).unwrap();

                let &(ref lock, ref cvar) = &*pair;
                let mut buf = lock.lock().unwrap();

                for pkt_index in 0..7*TS_PKT_SZ/TS_PKT_SZ {
                    let ts_pkt_raw_src = &pkts_raw[pkt_index*TS_PKT_SZ .. (pkt_index+1)*TS_PKT_SZ];

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
        let mut buf = lock.lock().unwrap();

        buf = cvar.wait(buf).unwrap();

        while !buf.is_empty() {
            let ts_pkt_raw = buf.pop_front().unwrap();

            let res = parse_ts_single(&ts_pkt_raw);
            match res {
                Ok((_, (_, ts_header, _))) => {
                    println!("pid: 0x{:04X}/{}, cc: {}", ts_header.pid, ts_header.pid, ts_header.cc);
                },
                _  => {
                    println!("error or incomplete cap: {:?}, len: {:?}, data: 0x{:02X?}{:02X?}{:02X?}",
                        buf.capacity(), buf.len(), ts_pkt_raw[0], ts_pkt_raw[1], ts_pkt_raw[2]);
                }
            }
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
        InputFile{
            url: url,
        }
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

impl <I> Wrkr<I>
    where I: Input + std::marker::Send + 'static
{
    pub fn new(input: I) -> Wrkr<I> {
        Wrkr{
            input: Arc::new(Mutex::new(input)),
        }
    }

    pub fn run(&self) -> Result<()> {
        let input = self.input.clone();
        {
            try!(input.lock().unwrap().open());
        }

        thread::spawn(move || {
            loop {
                input.lock().unwrap().read();
            }
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
        .arg(Arg::with_name("input")
            // .index(1)
            .short("i")
            .long("input")
            .help("Sets the input file to use")
            .required(true)
            .takes_value(true))
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
    let input_udp = InputUDP::new(input_url_1, 5000*7);
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
