import "./styles/index.css"
import React from "react"
import ReactDOM from "react-dom"
import ReconnectingWebSocket from "reconnectingwebsocket"
import msgpack from "msgpack-lite"
import blobToBuffer from "blob-to-buffer"

import AtomWrapper from "./entities/atoms/atom-wrapper"
import store from "./store"
import {wsAtomsAddSingle} from "./actions"
import App from "./components/app"

const wsHost = window.location.hostname
let wsPort = 8000
if (__PROD__) { wsPort =  window.location.port }
const wsURL = `ws://${wsHost}:${wsPort}/ws/v1`

function wsOnOpen() { }

function wsOnMessage(event) {
  blobToBuffer(event.data, (_, buffer) => {
    const msg = msgpack.decode(buffer)

    const atom = AtomWrapper.fromMessagePack(msg)

    store.dispatch(wsAtomsAddSingle(atom))
  })
}

function wsOnClose() { }

const ws = new ReconnectingWebSocket(wsURL)
ws.reconnectInterval = 2000
ws.onopen = wsOnOpen
ws.onmessage = wsOnMessage
ws.onclose = wsOnClose

function render(store) {
  const root = document.getElementById("root")

  if (__DEV__) {
    const RedBox = require("redbox-react").default
    try {
      ReactDOM.render(<App store={store}/>, root)
    } catch (e) {
      ReactDOM.render(<RedBox error={e} />, root)
    }
  } else {
    ReactDOM.render(<App store={store}/>, root)
  }
}

render(store)
