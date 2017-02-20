import React from 'react'
import ReactDOM from 'react-dom'
import {Provider} from 'react-redux'
import ReconnectingWebSocket from 'reconnectingwebsocket'
import msgpack from 'msgpack-lite'
import blobToBuffer from 'blob-to-buffer'

import H264NALSliceType from './h264-nal-slice-type'
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
    const message = msgpack.decode(buffer)

    // const id = message[0]
    // const offset = message[1]
    // const nal_type_raw = message[3][0][0]
    const sliceTypeRaw = message[3][1][0]
    const picParameterSetID = message[3][3]
    const frameNum = message[3][4]
    const picOrderCntLsb = message[3][5]

    const sliceType = H264NALSliceType.parse(sliceTypeRaw)

    const atom = {
      sliceType: sliceType,
      picParameterSetID: picParameterSetID,
      frameNum: frameNum,
      picOrderCntLsb: picOrderCntLsb
    }

    store.dispatch(atomsAdd(atom))
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
