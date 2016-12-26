mod method;
mod status;
mod request;
mod request_error;
mod response;
mod header;


const CR: u8 = 0x0D;
const LF: u8 = 0x0A;
const CCR: char = CR as char;
const CLF: char = LF as char;

const HEADER_CONTENT_LENGTH : &'static str = "Content-Length";


pub use self::method::Method;
pub use self::status::Status;
pub use self::request::Request;
pub use self::request_error::RequestError;
pub use self::response::Response;
pub use self::header::Header;
