const {resolve} = require('path')
const express = require('express')
const webpack = require('webpack')
const compiler = require('./tasks/webpack/compiler')()
const webpackConfig = require('./tasks/webpack/config')

const app = express()
const router = express.Router()

router.get('/', (req, res) => {
  res.sendFile(__dirname + '/html/index.html')
})

app.use(router)
app.use(require("webpack-dev-middleware")(compiler, {
    noInfo: true,
    publicPath: '/static'
}));
app.use(require("webpack-hot-middleware")(compiler, {
  log: console.log,
  path: '/__webpack_hmr',
  heartbeat: 10 * 1000
}));

app.listen(8000)
