// message sent through BUS
pub struct Message {
    pub kind: Kind,
    pub body: Body,
}

pub enum Kind {
    // broadcast data across all connected
    // websocket clients
    WsBroadcast,

    // shutdown server
    Shutdown,
}

pub enum Body {
    Text(String),
    Binary(Vec<u8>),
}
