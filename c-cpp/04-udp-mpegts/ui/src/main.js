import React from 'react'
import ReactDOM from 'react-dom'
import ReconnectingWebSocket from 'reconnectingwebsocket'
import msgpack from 'msgpack-lite'
import blobToBuffer from 'blob-to-buffer'

import {H264NALSliceType} from './h264-nal-slice-type.js'

console.log(H264NALSliceType)

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

    // const nal_type_raw = message[0][0]
    const slice_type_raw = message[1][0]
    // const pic_parameter_set_id = message[3]
    const frame_num = message[4]
    const pic_order_cnt_lsb = message[5]

    const sliceType = H264NALSliceType.parse(slice_type_raw)

    console.log(sliceType.toStringSimple(), frame_num, pic_order_cnt_lsb)
  })
}

function wsOnClose() {
  console.log('ws-on-close')
}

const ws = new ReconnectingWebSocket(wsURL)
ws.binaryType = 'arraybuffer'
ws.reconnectInterval = 1000
ws.onopen = wsOnOpen
ws.onmessage = wsOnMessage
ws.onclose = wsOnClose

function render() {
  ReactDOM.render(
    <h1>libVA UI</h1>,
    document.getElementById('root')
  )
}

render()
