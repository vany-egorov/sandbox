use std::env;
use std::process;
use std::net::{UdpSocket, Ipv4Addr};
use nom::IResult;

#[macro_use]
extern crate nom;


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


pub fn ts_sync_byte(input:&[u8]) -> IResult<&[u8], (u8, &[u8])> {
  do_parse!(input,
    sync_byte: tag!(&[0x47]) >>
    bytes:  take!(187)       >>

    (*sync_byte.first().unwrap(), bytes)
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
    let args: Vec<String> = env::args().collect();
    println!("{:?}", args);

    let port = 5500;
    let addr = Ipv4Addr::new(239, 255, 1, 1);
    // let addr = Ipv4Addr::new(127, 0, 0, 1);
    let iface = Ipv4Addr::new(0, 0, 0, 0);
    let socket = match UdpSocket::bind((addr, port)) {
        Ok(v) => v,
        Err(err) => {
            eprintln!("error socket-bind({:?}, {:?}): {:?}\n", addr, port, err);
            process::exit(1);
        }
    };
    if let Err(err) = socket.join_multicast_v4(&addr, &iface) {
        eprintln!("error join-multicast-v4({:?}, {:?}): {:?}\n", addr, port, err);
        process::exit(1);
    };

    // named!(
    //     ts_parser<&[u8], (&[u8], u8, u8, u8, u16)>,
    //     tuple!(
    //         tag!(&[0x47]),
    //         bits!(take_bits!(u8, 1)),
    //         bits!(take_bits!(u8, 1)),
    //         bits!(take_bits!(u8, 1)),
    //         bits!(take_bits!(u16, 13))
    //     )
    // );

    loop {
        // read from the socket
        let mut buf = [0; 1316];
        let (_, _) = socket.recv_from(&mut buf).unwrap();

        named!(
            ts<&[u8], Vec<(u8, &[u8])>>,
            many1!(ts_sync_byte)
        );

        let res1 = ts(&buf);
        match res1 {
          IResult::Done(_, data) => {
            for &(_, ts_data1) in data.iter() {
                let res2 = parse_ts_header(ts_data1);
                match res2 {
                    IResult::Done(_, ts_header) => {
                        println!("pid: 0x{:04X}/{}, cc: {}", ts_header.pid, ts_header.pid, ts_header.cc);
                    },
                    _  => {
                        println!("error or incomplete");
                        panic!("cannot parse header");
                    }
                }
            }
          },
          _  => {
            println!("error or incomplete");
            panic!("cannot parse header");
          }
        }

        // match ts_parser(&buf) {
        //     IResult::Done(tail, (_, _, _, _, pid)) => {
        //         println!("[<] MPEGTS / {:4}; pid: {:5}/0x{:04X}; buf: 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}; tail: 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}",
        //             sz,
        //             pid, pid,

        //             buf[0], buf[1], buf[2], buf[3],

        //             tail[0], tail[1], tail[2], tail[3],
        //             tail[4], tail[5], tail[6], tail[7],
        //             tail[8], tail[9], tail[10], tail[11],
        //         );
        //     },
        //     IResult::Error(e) => {
        //         println!("error parsing: {:?}", e);
        //     },
        //     IResult::Incomplete(needed) => {
        //         println!("incomplete, needed: {:?}", needed);
        //     },
        // }
    }
}
