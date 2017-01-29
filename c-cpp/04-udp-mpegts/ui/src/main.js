import React from 'react'
import ReactDOM from 'react-dom'
import ReconnectingWebSocket from 'reconnectingwebsocket'

const wsURL = `ws://${window.location.hostname}:${window.location.port}/ws/v1`

function wsOnOpen() {
  console.log('ws-on-open')
}

function wsOnMessage() {
  console.log('ws-on-message')
}

function wsOnClose() {
  console.log('ws-on-close')
}

const ws = new ReconnectingWebSocket(wsURL)
ws.reconnectInterval = 1000
ws.onopen = wsOnOpen
ws.onmessage = wsOnMessage
ws.onclose = wsOnClose

ReactDOM.render(
  <h1>Hello, world!</h1>,
  document.getElementById('root')
)
