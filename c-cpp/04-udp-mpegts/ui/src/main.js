import React from 'react'
import ReactDOM from 'react-dom'
import {Provider} from 'react-redux'
import ReconnectingWebSocket from 'reconnectingwebsocket'
import msgpack from 'msgpack-lite'
import blobToBuffer from 'blob-to-buffer'

import AtomWrapper from './entities/atoms/atom-wrapper'
import store from './store'
import {atomsAdd} from './actions'
import App from './components/app'

const wsHost = window.location.hostname
// const wsPort = window.location.port
const wsPort = 8000
const wsURL = `ws://${wsHost}:${wsPort}/ws/v1`

function wsOnOpen() {
  console.log('ws-on-open')
}

function wsOnMessage(event) {
  blobToBuffer(event.data, (_, buffer) => {
    const msg = msgpack.decode(buffer)

    const atomWrapper = AtomWrapper.fromMessagePack(msg)

    store.dispatch(atomsAdd(atomWrapper))
  })
}

function wsOnClose() {
  console.log('ws-on-close')
}

const ws = new ReconnectingWebSocket(wsURL)
ws.reconnectInterval = 2000
ws.onopen = wsOnOpen
ws.onmessage = wsOnMessage
ws.onclose = wsOnClose

function render(store) {
  ReactDOM.render(
    <Provider store={store}>
      <App />
    </Provider>,
    document.getElementById('root')
  )
}

render(store)
