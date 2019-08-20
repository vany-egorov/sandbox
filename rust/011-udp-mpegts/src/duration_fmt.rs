use std::fmt;
use std::time::Duration;

pub struct DurationFmt(pub Duration);

impl DurationFmt {
    #[inline(always)]
    fn duration(&self) -> Duration { self.0 }

    #[inline(always)]
    fn pure_nanos(&self) -> u128 {
        self.0.as_nanos() % Duration::from_micros(1).as_nanos()
    }

    #[inline(always)]
    fn pure_micros(&self) -> u128 {
        (self.0.as_nanos() % Duration::from_millis(1).as_nanos()) / Duration::from_micros(1).as_nanos()
    }

    #[inline(always)]
    fn pure_millis(&self) -> u128 {
        (self.0.as_nanos() % Duration::from_secs(1).as_nanos()) / Duration::from_millis(1).as_nanos()
    }

    #[inline(always)]
    fn pure_secs_as_f64(&self) -> f64 {
        ((self.0.as_nanos() % Duration::from_secs(60).as_nanos()) as f64) / (Duration::from_secs(1).as_nanos() as f64)
    }

    #[inline(always)]
    fn pure_mins(&self) -> u128 {
        (self.0.as_nanos() % Duration::from_secs(60*60).as_nanos()) / Duration::from_secs(60).as_nanos()
    }

    #[inline(always)]
    fn pure_hours(&self) -> u128 {
        self.0.as_nanos() / Duration::from_secs(60*60).as_nanos()
    }
}

impl fmt::Debug for DurationFmt {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.duration() {
            d if d <= Duration::from_micros(1) => write!(f, "{}ns", self.pure_nanos()),
            d if d <= Duration::from_millis(1) => {
                let ns = self.pure_nanos();
                let mcs = self.pure_micros();

                match ns {
                    0 => write!(f, "{}µs", mcs),
                    _ => write!(f, "{}µs{}ns", mcs, ns),
                }
            },
            d if d <= (Duration::from_secs(1) / 10) => {
                let mcs = self.pure_micros();
                let ms = self.pure_millis();

                match mcs {
                    0 => write!(f, "{}ms", ms),
                    _ => write!(f, "{}ms{}µs", ms, mcs),
                }
            },
            _ => {
                let h = self.pure_hours();
                let m = self.pure_mins();
                let s = self.pure_secs_as_f64();

                if h != 0 {
                    write!(f, "{}h", h)?;
                }

                if m != 0 {
                    write!(f, "{}m", m)?;
                }

                if s != 0.0 {
                    write!(f, "{:.2}s", s)
                } else {
                    Ok(())
                }
            },
        }
    }
}
