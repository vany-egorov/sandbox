import "./styles/index.css"
import React from "react"
import ReactDOM from "react-dom"
import ReconnectingWebSocket from "reconnectingwebsocket"
import msgpack from "msgpack-lite"
import blobToBuffer from "blob-to-buffer"

import AtomWrapper from "./entities/atoms/atom-wrapper"
import store from "./store"
import {atomsAddSingle} from "./actions"
import App from "./components/app"

const wsHost = window.location.hostname
// const wsPort = window.location.port
const wsPort = 8000
const wsURL = `ws://${wsHost}:${wsPort}/ws/v1`

function wsOnOpen() { }

function wsOnMessage(event) {
  blobToBuffer(event.data, (_, buffer) => {
    const msg = msgpack.decode(buffer)

    const atom = AtomWrapper.fromMessagePack(msg)

    store.dispatch(atomsAddSingle(atom))
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
    <App store={store}/>,
    document.getElementById("root")
  )
}

render(store)
