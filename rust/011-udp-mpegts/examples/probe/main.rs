extern crate ts;

mod error;

extern crate clap;
extern crate url;

use std::io::{Cursor, Write};
use std::collections::VecDeque;
use std::net::{Ipv4Addr, UdpSocket};
use std::process;
use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::Duration;
use url::{Host, Url};

use error::{Error, Kind as ErrorKind, Result};

use clap::{App, Arg};

// pub struct TSPESOptionalHeader {
//     // marker_bits              :2
//     // scrambling_control       :2
//     // priority                 :1
//     // data_alignment_indicator :1
//     // copyright                :1
//     // original_or_copy         :1
//     b1: u8,

//     // PTS_DTS_indicator         :2
//     // ESCR_flag                 :1
//     // ES_rate_flag              :1
//     // DSM_trick_mode_flag       :1
//     // additional_copy_info_flag :1
//     // CRC_flag                  :1
//     // extension_flag            :1
//     b2: u8,

//     header_length: u8,

//     dts: Option<u64>,
//     pts: Option<u64>,
// }

// pub struct TSPES {
//     stream_id: u8,

//     packet_length: u16,

//     header: Option<TSPESOptionalHeader>,
// }

// impl TSPES {
//     fn new() -> TSPES {
//         TSPES {
//             stream_id: 0,
//             packet_length: 0,
//             header: None,
//         }
//     }

//     fn parse(input: &[u8]) -> IResult<&[u8], TSPES> {
//         let mut p = TSPES::new();

//         #[cfg_attr(rustfmt, rustfmt_skip)]
//         let(input, (sid, p_len)) = try!(do_parse!(input,
//             _start_code: tag!(&[0x00, 0x00, 0x01])  // TODO: move to PESStartCode as const

//             >> b1: bits!(take_bits!(u8, 8))

//             >> b2: bits!(take_bits!(u8, 8))
//             >> b3: bits!(take_bits!(u8, 8))

//             >> (
//                 b1,
//                 ((b2 as u16) << 8) | b3 as u16
//             )
//         ));

//         p.stream_id = sid;
//         p.packet_length = p_len;

//         println!(
//             "[t] [PES] (:stream-id {} :packet-length {})",
//             p.stream_id, p.packet_length
//         );

//         Ok((input, p))
//     }
// }

pub struct TS {
    pmt_pid: Option<u16>,

    eit_buf: Cursor<Vec<u8>>,
}

impl TS {
    fn new() -> TS {
        TS {
            pmt_pid: None,

            eit_buf: Cursor::new(Vec::with_capacity(2048)),
        }
    }
}

// struct DemuxerTS {}

// impl DemuxerTS {
//     pub fn new() -> DemuxerTS {
//         DemuxerTS {}
//     }
// }

trait Input {
    fn open(&mut self) -> Result<()>;
    fn read(&mut self) -> Result<()>;
    fn close(&mut self) -> Result<()>;
}

// trait Filter {
//     fn consume_strm(&self);
//     fn consume_trk(&self);
//     fn consume_pkt_raw(&self);
//     fn consume_pkt(&self);
//     fn consume_frm(&self);

//     fn produce_strm(&self);
//     fn produce_trk(&self);
//     fn produce_pkt_raw(&self);
//     fn produce_pkt(&self);
//     fn produce_frm(&self);
// }

struct InputUDP {
    url: Url,

    // circullar-buffer / fifo
    buf: Arc<(Mutex<VecDeque<[u8; ts::Packet::SZ]>>, Condvar)>,

    ts: TS,
}

impl InputUDP {
    pub fn new(url: Url, buf_cap: usize) -> InputUDP {
        InputUDP {
            url: url,
            buf: Arc::new((Mutex::new(VecDeque::with_capacity(buf_cap)), Condvar::new())),

            ts: TS::new(),
        }
    }

    fn demux(&mut self, ts_pkt_raw: &[u8]) -> Result<()> {
        let pkt = ts::Packet::new(&ts_pkt_raw)?;

        if let Some(pcr) = pkt.pcr()? {
            println!("{:?}", pcr);
        }

        if let Some(pat) = pkt.pat()? {
            println!("{:?}", pat);

            if let Some(pid) = pat.first_program_map_pid() {
                self.ts.pmt_pid = Some(pid);
            }
        }

        if let Some(pid) = self.ts.pmt_pid {
            if let Some(pmt) = pkt.pmt(pid)? {
                println!("{:?}", pmt);
            }
        }

        if let Some(sdt) = pkt.sdt()? {
            println!("{:?}", sdt);
        }

        if let Some(buf) = pkt.eit()? {
            if pkt.pusi() {
                if self.ts.eit_buf.position() != 0 {
                    let eit = ts::EIT::new(self.ts.eit_buf.get_ref().as_slice());
                    println!("{:?}", eit);
                }

                self.ts.eit_buf.set_position(0);
                self.ts.eit_buf.get_mut().clear();
            }

            self.ts.eit_buf.write_all(buf)?;
        }

        Ok(())
    }
}

impl Input for InputUDP {
    fn open(&mut self) -> Result<()> {
        let input_host = self
            .url
            .host()
            .ok_or(Error::new(ErrorKind::InputUrlMissingHost))?;

        let input_port = self.url.port().unwrap_or(5500);

        let input_host_domain = match input_host {
            Host::Domain(v) => Ok(v),
            _ => Err(Error::new(ErrorKind::InputUrlHostMustBeDomain)),
        }?;

        let iface = Ipv4Addr::new(0, 0, 0, 0);
        // let socket = try!(UdpSocket::bind((input_host_domain, input_port)));;

        // let iface = Ipv4Addr::new(127, 0, 0, 1);
        println!(
            "[<] {:?}: {:?} @ {:?}",
            input_host_domain, input_port, iface
        );

        let input_host_ip_v4: Ipv4Addr = input_host_domain.parse().unwrap();

        let socket = UdpSocket::bind((input_host_domain, input_port))?;

        if let Err(e) = socket.join_multicast_v4(&input_host_ip_v4, &iface) {
            eprintln!("error join-multiocast-v4: {}", e);
        }

        let pair = self.buf.clone();
        thread::spawn(move || {
            let mut ts_pkt_raw: [u8; ts::Packet::SZ] = [0; ts::Packet::SZ];

            loop {
                // MTU (maximum transmission unit) == 1500 for Ethertnet
                // 7*ts::Packet::SZ = 7*188 = 1316 < 1500 => OK
                let mut pkts_raw = [0; 7 * ts::Packet::SZ];
                let (_, _) = socket.recv_from(&mut pkts_raw).unwrap();

                let &(ref lock, ref cvar) = &*pair;
                let mut buf = match lock.lock() {
                    Err(e) => {
                        eprintln!("lock and get buffer failed: {}", e);
                        continue
                    },
                    Ok(buf) => buf,
                };

                for pkt_index in 0..7 * ts::Packet::SZ / ts::Packet::SZ {
                    let ts_pkt_raw_src = &pkts_raw[pkt_index * ts::Packet::SZ..(pkt_index + 1) * ts::Packet::SZ];

                    // println!("#{:?} -> [{:?} .. {:?}]; src-len: {:?}, dst-len: {:?}",
                    //     pkt_index, pkt_index*ts::Packet::SZ, (pkt_index+1)*ts::Packet::SZ,
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
        let mut buf = lock
            .lock()
            .ok()
            .ok_or(Error::new_with_details(ErrorKind::SyncPoison, "udp read lock error"))?;

        buf = cvar.wait(buf).ok().ok_or(Error::new_with_details(
            ErrorKind::SyncPoison,
            "udp read cwar wait erorr"
        ))?;

        while !buf.is_empty() {
            let ts_pkt_raw = buf.pop_front().unwrap();

            if let Err(e) = self.demux(&ts_pkt_raw) {
                eprintln!("error demux ts-packet: ({:?})", e);
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
        InputFile { url: url }
    }
}

impl Input for InputFile {
    fn open(&mut self) -> Result<()> {
        println!("<<< File open {}", self.url);

        Ok(())
    }

    fn read(&mut self) -> Result<()> {
        thread::sleep(Duration::from_secs(1000));

        Ok(())
    }

    fn close(&mut self) -> Result<()> {
        println!("<<< File close {}", self.url);

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
            input.lock().unwrap().open()?;
        }

        thread::spawn(move || loop {
            match input.lock().unwrap().read() {
                Err(err) => {
                    eprintln!("error read {}", err);
                    return
                },
                Ok(_) => {},
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
