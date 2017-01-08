const {resolve} = require('path')
const webpack = require('webpack')
const ProgressBarPlugin = require('progress-bar-webpack-plugin');


const config = {
  devtool: 'source-map',
  entry: [
    // The script refreshing the browser on none hot updates
    'webpack-hot-middleware/client?reload=true',

    resolve('src/main.js')
  ],
  output: {
    filename: resolve('static/main.js')
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
