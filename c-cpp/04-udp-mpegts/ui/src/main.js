import './styles/index.css'
import React from 'react'
import ReactDOM from 'react-dom'
import {Provider} from 'react-redux'
import ReconnectingWebSocket from 'reconnectingwebsocket'
import msgpack from 'msgpack-lite'
import blobToBuffer from 'blob-to-buffer'
import Bacon from 'baconjs'

import AtomWrapper from './entities/atoms/atom-wrapper'
import store from './store'
import {atomsAddMulti} from './actions'
import App from './components/app'

const wsHost = window.location.hostname
// const wsPort = window.location.port
const wsPort = 8000
const wsURL = `ws://${wsHost}:${wsPort}/ws/v1`

const bus = new Bacon.Bus()
bus
  .bufferWithTimeOrCount(500, 30)
  .onValue((atoms) => {
    store.dispatch(atomsAddMulti(atoms))
  })

function wsOnOpen() { }

function wsOnMessage(event) {
  blobToBuffer(event.data, (_, buffer) => {
    const msg = msgpack.decode(buffer)

    const atomWrapper = AtomWrapper.fromMessagePack(msg)
    bus.push(atomWrapper)
  })
}

function wsOnClose() { }

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
