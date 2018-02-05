#[macro_use]
extern crate nom;
extern crate url;
extern crate clap;

use std::process;
use std::time::Duration;
use std::collections::VecDeque;
use std::net::{UdpSocket, Ipv4Addr};
use std::thread;
use nom::IResult;
use clap::{Arg, App};
use url::{Url, Host/*, ParseError*/};


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

    // adaption-field-control
    // :1
    afc: u8,

    // :1
    contains_payload: u8,

    // continuity-counter
    // :4
    cc: u8
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

struct CircularBuffer {
}

struct InputUDP {
}

impl InputUDP {
    pub fn new() -> InputUDP {
        InputUDP{}
    }

    pub fn start(&self) {
        thread::spawn(move || {
            loop {
                thread::sleep(Duration::from_millis(1000));
                println!("< udp");
            }
        });
    }
}

struct DemuxerTS {
}

impl DemuxerTS {
    pub fn new() -> DemuxerTS {
        DemuxerTS{}
    }

    pub fn start(&self) {
        thread::spawn(move || {
            loop {
                thread::sleep(Duration::from_millis(1000));
                println!("< mpegts");
            }
        });
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
        .about("Video/audio manipulation tool")
        .arg(Arg::with_name("input")
            // .index(1)
            .short("i")
            .long("input")
            .help("Sets the input file to use")
            .required(true)
            .takes_value(true))
        .get_matches();

    let input_udp = InputUDP::new();
    let demuxer_ts = DemuxerTS::new();

    input_udp.start();
    demuxer_ts.start();

    let input_raw = matches.value_of("input").unwrap();
    let input = match Url::parse(input_raw) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("error parse input url: {:?}\n", e);
            process::exit(1);
        }
    };
    let input_host = match input.host() {
        Some(v) => v,
        _ => {
            eprintln!("expected input host\n");
            process::exit(1);
        }
    };
    let input_port = match input.port() {
        Some(v) => v,
        _ => 5500,
    };

    let input_host_ip_v4 = match input_host {
        Host::Ipv4(v) => v,
        _ => {
            eprintln!("expected ipv4 host({:?})\n", input_host);
            process::exit(1);
        }
    };

    let iface = Ipv4Addr::new(0, 0, 0, 0);
    let socket = match UdpSocket::bind((input_host_ip_v4, input_port)) {
        Ok(v) => v,
        Err(err) => {
            eprintln!("error socket-bind({:?}, {:?}): {:?}\n", input_host_ip_v4, input_port, err);
            process::exit(1);
        }
    };
    if let Err(err) = socket.join_multicast_v4(&input_host_ip_v4, &iface) {
        eprintln!("error join-multicast-v4({:?}, {:?}): {:?}\n", input_host_ip_v4, input_port, err);
        process::exit(1);
    };

    // 188*7 = 1316
    let mut ts_pkt_raw: [u8; TS_PKT_SZ] = [0; TS_PKT_SZ];
    let mut fifo: VecDeque<[u8; TS_PKT_SZ]> = VecDeque::with_capacity(100*7); // TODO: move initial capacity to config

    loop {
        // read from the socket
        let mut buf = [0; 7*TS_PKT_SZ];
        let (_, _) = socket.recv_from(&mut buf).unwrap();

        for pkt_index in 0..7*TS_PKT_SZ/TS_PKT_SZ {
            let ts_pkt_raw_src = &buf[pkt_index*TS_PKT_SZ .. (pkt_index+1)*TS_PKT_SZ];

            // println!("#{:?} -> [{:?} .. {:?}]; src-len: {:?}, dst-len: {:?}",
            //     pkt_index, pkt_index*TS_PKT_SZ, (pkt_index+1)*TS_PKT_SZ,
            //     ts_pkt_raw_src.len(), ts_pkt_raw.len(),
            // );

            ts_pkt_raw.copy_from_slice(ts_pkt_raw_src);
            fifo.push_back(ts_pkt_raw);
        }

        while !fifo.is_empty() {
            ts_pkt_raw = fifo.pop_front().unwrap();

            let res = parse_ts_single(&ts_pkt_raw);
            match res {
                IResult::Done(_, (_, ts_header, _)) => {
                    // println!("pid: 0x{:04X}/{}, cc: {}", ts_header.pid, ts_header.pid, ts_header.cc);
                },
                _  => {
                    println!("error or incomplete");
                    panic!("cannot parse header");
                }
            }
        }
    }
}
