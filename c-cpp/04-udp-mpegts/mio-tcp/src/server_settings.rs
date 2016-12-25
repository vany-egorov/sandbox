

#[derive(Debug, Clone, Copy)]
pub struct ServerSettings {
    // Slab (token + connection storge) capacity
    pub max_connections: usize,

    // mio::channel::sync_channel
    // max_connections * 1000 => 1000 messages per connection
    pub sync_channel_bound: usize,

    // mio::Events capacity
    pub events_cap: usize,
}

impl ServerSettings {
    fn new() -> ServerSettings {
        ServerSettings::default()
    }
}

impl Default for ServerSettings {
    fn default() -> ServerSettings {
        ServerSettings {
            max_connections: 100,
            sync_channel_bound: 10_000,
            events_cap: 1024,
        }
    }
}
