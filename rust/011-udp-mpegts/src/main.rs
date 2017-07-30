use std::net::{UdpSocket, Ipv4Addr};

fn main() {
    let port = 5500;
    let addr = Ipv4Addr::new(239, 1, 1, 1);
    let iface = Ipv4Addr::new(0, 0, 0, 0);
    let socket = UdpSocket::bind((addr, port)).unwrap();
    socket.join_multicast_v4(&addr, &iface).unwrap();

    loop {
        // read from the socket
        let mut buf = [0; 1316];
        let (sz, _) = socket.recv_from(&mut buf).unwrap();
        println!("[<] {}: {:02X} {:02X} {:02X} {:02X}",
            sz, buf[0], buf[1], buf[2], buf[3]);
    }
}
