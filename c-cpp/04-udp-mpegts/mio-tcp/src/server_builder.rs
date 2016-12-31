use server::Server;
use result::Result;
use router::Router;
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

    pub fn finalize<R>(&self, router: R) -> Result<Server<R>>
        where R: Router
    {
        Server::new(self.settings, router)
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
