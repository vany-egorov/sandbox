/* global configuration file */

const config = {
  env: process.env.NODE_ENV || 'development',
  server: {
    host: "0.0.0.0",
    port: process.env.PORT || 1024
  },
}

module.exports = config
