import React from 'react'
import ReactDOM from 'react-dom'
import ReconnectingWebSocket from 'reconnectingwebsocket'
import msgpack from 'msgpack-lite'
import blobToBuffer from 'blob-to-buffer'

console.log(msgpack)
console.log('==>')

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

    let slice_type = ''
    switch (slice_type_raw) {
    case 2:
    case 4:
    case 7:
    case 9: {
      slice_type = 'I'
      break
    }

    case 0:
    case 3:
    case 5:
    case 8: {
      slice_type = 'P'
      break
    }

    case 1:
    case 6: {
      slice_type = 'B'
      break
    }
    }

    console.log(slice_type, frame_num, pic_order_cnt_lsb)
  })

  // const options = {codec: msgpack.createCodec({useraw: true})}
  // const arr = new Uint8Array(event.data)
  // const buf = arr.buffer
  // const data = msgpack.decode(new Uint8Array(buf), options)
  // console.log(data)
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

ReactDOM.render(
  <h1>libVA UI</h1>,
  document.getElementById('root')
)
