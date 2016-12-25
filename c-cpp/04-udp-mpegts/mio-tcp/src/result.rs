use std::result::Result as StdResult;

use error::Error;


pub type Result<T> = StdResult<T, Error>;
