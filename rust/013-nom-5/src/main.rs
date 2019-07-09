#[macro_use]
extern crate nom;

use nom::IResult;

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSHeader {
    tei: u8,
    pusi: u8,
    tp: u8,
    pid: u16,
}

#[allow(dead_code)]
#[derive(Debug)]
pub struct TSAdaptation {
    afl: u8,
    di: u8,
    rai: u8,
    espi: u8,
    pcr_flag: u8,
    opcr_flag: u8,
    spf: u8,
    tpdf: u8,
    afef: u8,
}

pub fn parse_ts_header(input: &[u8]) -> IResult<&[u8], TSHeader> {
    do_parse!(
        input,
        b1: bits!(tuple!(
            take_bits!(1u8),
            take_bits!(1u8),
            take_bits!(1u8),
            take_bits!(5u8)
        )) >> b2: bits!(take_bits!(8u8))
            >> (TSHeader {
                tei: b1.0,
                pusi: b1.1,
                tp: b1.2,
                pid: ((b1.3 as u16) << 8) | b2 as u16,
            })
    )
}

pub fn parse_ts_adaptation(input: &[u8]) -> IResult<&[u8], TSAdaptation> {
    do_parse!(
        input,
        b1: bits!(take_bits!(8u8))
            >> b2: bits!(tuple!(
                take_bits!(1u8),
                take_bits!(1u8),
                take_bits!(1u8),
                take_bits!(1u8),
                take_bits!(1u8),
                take_bits!(1u8),
                take_bits!(1u8),
                take_bits!(1u8)
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
            })
    )
}

fn main() {
    let input = [1u8, 2u8, 3u8];

    let ts_header = parse_ts_header(&input).unwrap();
    println!("(:ts-header {:?})", ts_header);

    let ts_adaptation = parse_ts_adaptation(&input).unwrap();
    println!("(:ts-adaptation {:?})", ts_adaptation);
}
