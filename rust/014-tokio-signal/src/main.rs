use std::error::Error as StdError;
use std::fmt;

use futures::compat::Stream01CompatExt;
use futures::{select, StreamExt};
use futures_legacy::prelude::*;
use tokio_signal::unix::{Signal, SIGINT};

struct Error {}

impl StdError for Error {
    fn description(&self) -> &str {
        ""
    }

    fn cause(&self) -> Option<&dyn StdError> {
        None
    }
}

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "")
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "")
    }
}

#[runtime::main(runtime_tokio::Tokio)]
async fn main() -> Result<(), Error> {
    println!("tokio-signal started");

    let mut ctrl_c = Signal::new(SIGINT).flatten_stream().compat().fuse();

    loop {
        select! {
            _ = ctrl_c.next() => {
                println!("==> got SIGINT");
                break;
            }
        }
    }

    println!("==> will gracefully shutdown");

    Ok(())
}
