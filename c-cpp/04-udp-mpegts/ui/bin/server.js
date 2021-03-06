const {resolve} = require("path")
const express = require("express")
const proxy = require("http-proxy-middleware")
const webpack = require("webpack")
const compiler = require("./tasks/webpack/compiler")()
const config = require("./config")
const webpackConfig = require("./tasks/webpack/config")

const app = express()
const router = express.Router()

const proxyOptions = {
  target: `http://${config.serverBackend.host}:${config.serverBackend.port}`,
  changeOrigin: true,
  ws: true,
  logLevel: "debug"
}
const p = proxy("/ws", proxyOptions)

router.get("/", (req, res) => {
  res.sendFile(__dirname + "/html/index.html")
})

app.use(router)
app.use(p)
app.use(require("webpack-dev-middleware")(compiler, {
    noInfo: true,
    publicPath: "/static"
}));
app.use(require("webpack-hot-middleware")(compiler, {
  log: console.log,
  path: "/__webpack_hmr",
  heartbeat: 10 * 1000
}));

const server = app.listen(config.serverLocal.port)
// server.on("upgrade", p.upgrade)
