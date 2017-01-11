const {resolve} = require('path')
const webpack = require('webpack')
const ProgressBarPlugin = require('progress-bar-webpack-plugin');


const config = {
  devtool: 'source-map',
  entry: [
    // The script refreshing the browser on none hot updates
    'webpack-hot-middleware/client?reload=true&overlay=true',

    resolve('src/main.js')
  ],
  output: {
    path: resolve('static'),
    publicPath: '/',
    filename: 'main.js'
  },
  module: {
    loaders: [
      {
        test: /\.js$/,
        exclude: /(node_modules|bower_components)/,
        loaders: [
          'babel-loader',
          'eslint-loader'
        ]
      }
    ]
  },
  eslint: {
    configFile: resolve('.eslintrc')
  },
  plugins: [
    new ProgressBarPlugin(),
    new webpack.optimize.OccurenceOrderPlugin(),
    // Webpack 2.0 fixed this mispelling
    // new webpack.optimize.OccurrenceOrderPlugin(),
    new webpack.HotModuleReplacementPlugin(),
    new webpack.NoErrorsPlugin()
    // new webpack.optimize.UglifyJsPlugin({
    //   compress: {
    //     unused: true,
    //     dead_code: true,
    //     warnings: false
    //   }
    // })
  ]
}


module.exports = config
