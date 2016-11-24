var WS_SCHEMA = "ws://";
var WS_PATH = "/feed";
var WS_URL = WS_SCHEMA
  + window.location.hostname
  + ":"
  + window.location.port
  + WS_PATH;


var ws = new WebSocket(WS_URL);

function wsOnOpen() {
  console.log("open");
}

function wsOnMessage(e) {
  console.log("message \""+ e.data + "\"");
  ws.send("hello from client!");
}

function wsOnClose() {
  console.log("close");
}

ws.onopen = wsOnOpen;
ws.onmessage = wsOnMessage;
ws.onclose = wsOnClose;
