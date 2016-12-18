(function() {
  var observer = new MutationObserver(function() {
    if (document.body) {


      var location = window.location;
      var ws_path = "/ws/v1";
      var ws_url = "ws://"
        + location.hostname
        + ":"
        + location.port
        + ws_path;

      console.log(ws_url);
      var socket = new WebSocket(ws_url);


      observer.disconnect();
    }
  });
  observer.observe(document.documentElement, {childList: true});
})();
