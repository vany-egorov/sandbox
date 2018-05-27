mod error;

extern crate tokio;
extern crate tokio_signal;
extern crate futures;

use std::thread;
use std::process;
use tokio::io;
use tokio::net::{TcpListener, TcpStream};
use tokio::prelude::*;
use error::Result;


struct AppServer {
}

impl AppServer
{
    pub fn new() -> AppServer {
        AppServer{}
    }

    pub fn run(&self, addr: std::net::SocketAddr) -> Result<()> {
        let listener = TcpListener::bind(&addr).unwrap();

        let server = listener.incoming().for_each(|socket| {
            println!("[s] [+] addr={:?}", socket.peer_addr().unwrap());

            let connection = io::write_all(socket, "hello world\n")
                .then(|res| {
                    println!("[s] [>] {:?}", res);
                    Ok(())
                });

            // Spawn a new task that processes the socket:
            tokio::spawn(connection);

            Ok(())
        })
        .map_err(|err| {
            println!("[s] accept error = {:?}", err);
        });

        println!("[s] running on {:?}", addr);
        thread::spawn(move || {
            tokio::run(server);
        });

        Ok(())
    }
}

struct AppClient {
    id: u8,
}

impl AppClient
{
    pub fn new(id: u8) -> AppClient {
        AppClient{id: id}
    }

    pub fn run(&self, addr: &std::net::SocketAddr) -> Result<()> {
        let id = self.id;
        println!("[c/{}] try to connect", id);

        let tcp = TcpStream::connect(&addr);
        let client = tcp.and_then(move |socket| {
            println!("[c/{}] [+] addr={:?}", id, socket.peer_addr().unwrap());
            Ok(())
        }).map_err(|e| {
            println!("[c] error occurred: {}", e)
        });

        thread::spawn(move || {
            tokio::run(client);
        });

        Ok(())
    }
}

fn main() {
    let addr = "127.0.0.1:8000".parse().unwrap();

    let srv = AppServer::new();
    if let Err(err) = srv.run(addr) {
        eprintln!("error start server: {:?}\n", err);
        process::exit(1);
    }

    AppClient::new(1).run(&addr).unwrap();
    AppClient::new(2).run(&addr).unwrap();
    AppClient::new(3).run(&addr).unwrap();
    AppClient::new(4).run(&addr).unwrap();
    AppClient::new(5).run(&addr).unwrap();
    AppClient::new(6).run(&addr).unwrap();
    AppClient::new(7).run(&addr).unwrap();
    AppClient::new(8).run(&addr).unwrap();
    AppClient::new(9).run(&addr).unwrap();
    AppClient::new(10).run(&addr).unwrap();

    let ctrl_c = tokio_signal::ctrl_c().flatten_stream();
    let prog = ctrl_c.for_each(|()| {
        println!("SIGINT received - will non-graceful shutdown!");
        process::exit(0);

        Ok(())
    });
    tokio::run(prog.map_err(|err| panic!("{}", err)));
}
