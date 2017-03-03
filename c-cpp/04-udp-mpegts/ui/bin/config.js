/*
 * global development! configuration file
 * production configuration comes from production server
*/
const env = require("./common/env")
const {log} = require("./common/log")


const config = {
  env: env.parse(process.env.UI_ENV || "development"),

  // nodejs/express local server settings
  serverLocal: {
    host: "0.0.0.0",
    port: process.env.UI_PORT || 1024,
  },

  // real development backend server
  serverBackend: {
    host: "127.0.0.1",
    port: 8000,
  }
}

function toLog(v) {
  const p = ""  /* prefix */

  log("")
  log(`${p}env: "${env.toString(v.env)}"`)

  log(`${p}server-local(nodejs/express local server settings):`)
  log(`${p}  host: "${v.serverLocal.host}"`)
  log(`${p}  port: ${v.serverLocal.port}`)

  log(`${p}server-backend(real development backend server):`)
  log(`${p}  host: "${v.serverBackend.host}"`)
  log(`${p}  port: ${v.serverBackend.port}`)
  log("")
}

toLog(config)

module.exports = config
