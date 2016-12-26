use std::net::SocketAddr;

use mio::Poll;

use server::Server;
use result::Result;
use factory::Factory;
use server_settings::ServerSettings;


pub struct ServerBuilder {
    settings: ServerSettings,
}

impl ServerBuilder {
    pub fn new() -> ServerBuilder {
        ServerBuilder {
            settings: ServerSettings::default(),
        }
    }

    pub fn finalize<F>(&self, factory: F) -> Result<Server<F>>
        where F: Factory
    {
        Server::new(self.settings, factory)
    }

    pub fn with_settings(&mut self, settings: ServerSettings) -> &mut ServerBuilder {
        self.settings = settings;
        self
    }

    pub fn max_connections(&mut self, v: usize) -> &mut ServerBuilder {
        self.settings.max_connections = v;
        self
    }

    pub fn sync_channel_bound(&mut self, v: usize) -> &mut ServerBuilder {
        self.settings.sync_channel_bound = v;
        self
    }

    pub fn events_cap(&mut self, v: usize) -> &mut ServerBuilder {
        self.settings.events_cap = v;
        self
    }
}
