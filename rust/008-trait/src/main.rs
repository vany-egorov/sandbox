trait TCP {
    fn on_tcp_accept(&mut self);
}

trait HTTP {
    fn on_tcp_accept(&mut self) { println!("http -> on-tcp-accept"); }
    fn on_http_request(&mut self) { println!("http -> on-http-request"); }
    fn on_http_response(&mut self);
}

trait WS {
    fn on_tcp_accept(&mut self) {
        println!("ws -> on-tcp-accept");
    }

    fn on_http_request(&mut self) {
        println!("ws -> on-http-request");
    }

    fn on_http_response(&mut self) {
        println!("ws -> on-http-response");
    }

    fn on_ws_message(&mut self);
}


struct TCPHandler {
}

impl TCP for TCPHandler {
    fn on_tcp_accept(&mut self) { println!("tcp-handler -> on-tcp-accept") }
}

impl<F> TCP for F
    where F: FnMut()
{
    fn on_tcp_accept(&mut self) {
        self();
    }
}

struct HTTPHandler {
}

impl HTTP for HTTPHandler {
    fn on_http_response(&mut self) { println!("http-handler -> on-http-response") }
}

impl<F> HTTP for F
    where F: FnMut()
{
    fn on_http_response(&mut self) {
        self();
    }
}

struct WSHandler {
}

impl WS for WSHandler {
    fn on_ws_message(&mut self) { println!("ws-handler -> on-ws-message") }
}

impl<F> WS for F
    where F: FnMut()
{
    fn on_ws_message(&mut self) {
        self();
    }
}


enum Handler {
    TCP(Box<TCP>),
    HTTP(Box<HTTP>),
    WS(Box<WS>),
}


trait Factory {
    fn produce(&mut self) -> Handler;
}

impl<F> Factory for F
    where F: FnMut() -> Handler
{
    fn produce(&mut self) -> Handler {
        self()
    }
}


fn execute<F>(mut f: F) where F: Factory {
    let h = f.produce();

    match h {
        Handler::TCP(mut h) => {
            h.on_tcp_accept();
        },
        Handler::HTTP(mut h) => {
            h.on_http_response();
        },
        Handler::WS(mut h) => {
            h.on_ws_message();
        },
    }
}


fn main() {
    execute(|| {
        Handler::TCP(Box::new(TCPHandler{}))
    });
    execute(|| {
        Handler::TCP(Box::new(|| {
            println!("tcp-handler-closure -> on-tcp-accept");
        }))
    });

    execute(|| {
        Handler::HTTP(Box::new(HTTPHandler{}))
    });
    execute(|| {
        Handler::HTTP(Box::new(|| {
            println!("http-handler-closure -> on-http-response");
        }))
    });

    execute(|| {
        Handler::WS(Box::new(WSHandler{}))
    });
    execute(|| {
        Handler::HTTP(Box::new(|| {
            println!("ws-handler-closure -> on-ws-message");
        }))
    });
}
