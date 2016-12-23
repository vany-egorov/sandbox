use std::io::Cursor;

#[derive(Debug)]
enum State<T> {
    Disconnected,
    Connecting(Cursor<Vec<u8>>, Cursor<Vec<u8>>),
    WS(T),
    Finished,
}


fn main() {
    let mut state: State<u64> = State::Disconnected;
    state = State::Connecting(
        Cursor::new(Vec::with_capacity(2048)),
        Cursor::new(Vec::with_capacity(2048)),
    );

    match state {
        State::Connecting(_, _) => println!("connecting"),
        _ => println!("other"),
    }
}
