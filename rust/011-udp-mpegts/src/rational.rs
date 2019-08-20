pub const TB_27MHZ: Rational = Rational {
    num: 1,
    den: 27_000_000,
};
#[allow(dead_code)]
pub const TB_90KHZ: Rational = Rational {
    num: 1,
    den: 90_000,
};
#[allow(dead_code)]
pub const TB_1MS: Rational = Rational {
    num: 1,
    den: 1_000_000,
};
pub const TB_1NS: Rational = Rational {
    num: 1,
    den: 1_000_000_000,
};

pub struct Rational {
    num: u64,
    den: u64,
}

pub fn rescale(v: u64, src: Rational, dst: Rational) -> u64 {
    let num = (src.num as u128) * (dst.den as u128);
    let den = (src.den as u128) * (dst.num as u128);

    v * ((num / den) as u64)
}
