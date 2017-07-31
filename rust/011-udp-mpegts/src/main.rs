use std::net::{UdpSocket, Ipv4Addr};
use nom::IResult;

#[macro_use]
extern crate nom;


struct TSHeader {
    // transcport-error-indicator
    tei: bool,

    // payload-unit-start-indicator
    // Set when a PES, PSI, or DVB-MIP
    // packet begins immediately following the header.
    pusi: bool,

    transcport_priority: u8,
    pid: u16,
}


fn main() {
    let port = 5500;
    let addr = Ipv4Addr::new(239, 255, 1, 1);
    let iface = Ipv4Addr::new(0, 0, 0, 0);
    let socket = UdpSocket::bind((addr, port)).unwrap();
    socket.join_multicast_v4(&addr, &iface).unwrap();

    named!(
        ts_parser<&[u8], (&[u8], u8, u8, u8, u16)>,
        tuple!(
            tag!(&[0x47]),
            bits!(take_bits!(u8, 1)),
            bits!(take_bits!(u8, 1)),
            bits!(take_bits!(u8, 1)),
            bits!(take_bits!(u16, 13))
        )
    );

    loop {
        // read from the socket
        let mut buf = [0; 1316];
        let (sz, _) = socket.recv_from(&mut buf).unwrap();

        match ts_parser(&buf) {
            IResult::Done(tail, (_, _, _, _, pid)) => {
                println!("[<] MPEGTS / {:4}; pid: {:5}; buf: 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}; tail: 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}",
                    sz,
                    pid,

                    buf[0], buf[1], buf[2], buf[3],

                    tail[0], tail[1], tail[2], tail[3],
                    tail[4], tail[5], tail[6], tail[7],
                );
            },
            IResult::Error(e) => {
                println!("error parsing: {:?}", e);
            },
            IResult::Incomplete(needed) => {
                println!("incomplete, needed: {:?}", needed);
            },
        }
    }
}
