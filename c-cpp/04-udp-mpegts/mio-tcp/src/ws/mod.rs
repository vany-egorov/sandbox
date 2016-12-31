use sha1::Sha1;
use rustc_serialize;
use rustc_serialize::base64::ToBase64;


static GUID: &'static str = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


pub fn gen_key<S>(key: S) -> String
    where S: Into<String>
{
    let mut m = Sha1::new();

    m.update(key.into().as_bytes());
    m.update(GUID.as_bytes());

    return m.digest().bytes().to_base64(rustc_serialize::base64::STANDARD);
}
