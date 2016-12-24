struct Request {
}

struct Response {
}

mod ws {
    pub struct Message {
    }

    impl Message {
        pub fn new() -> Message {
            Message{}
        }
    }
}

pub use self::ws::Message as WsMessage;
