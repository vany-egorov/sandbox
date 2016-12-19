(function() {
  var observer = new MutationObserver(function() {
    if (document.body) {
      observer.disconnect();


      var location = window.location;
      var ws_path = "/ws/v1";
      var ws_url = "ws://"
        + location.hostname
        + ":"
        + location.port
        + ws_path;

      var socket = new WebSocket(ws_url);
      socket.onopen = function() {
        setInterval(function(){
          var timestamp = + new Date();
          socket.send("[" + timestamp + "] " + navigator.userAgent);
        }, 2000);
      };
    }
  });
  observer.observe(document.documentElement, {childList: true});
})();
