var JSWebrtc = {
  Player: null,
  VideoElement: null,
  CreateVideoElements: function() {
    var elements = document.querySelectorAll(".jswebrtc");
    for (var i = 0; i < elements.length; i++) {
      new JSWebrtc.VideoElement(elements[i]);
    }
  },
  FillQuery: function(query_string, obj) {
    obj.user_query = {};
    if (query_string.length == 0) return;
    if (query_string.indexOf("?") >= 0)
      query_string = query_string.split("?")[1];
    var queries = query_string.split("&");
    for (var i = 0; i < queries.length; i++) {
      var query = queries[i].split("=");
      obj[query[0]] = query[1];
      obj.user_query[query[0]] = query[1];
    }
    if (obj.domain) obj.vhost = obj.domain;
  },
  ParseUrl: function(rtmp_url) {
    var a = document.createElement("a");
    a.href = rtmp_url
      .replace("rtmp://", "http://")
      .replace("webrtc://", "http://")
      .replace("rtc://", "http://");
    var vhost = a.hostname;
    var app = a.pathname.substr(1, a.pathname.lastIndexOf("/") - 1);
    var stream = a.pathname.substr(a.pathname.lastIndexOf("/") + 1);
    app = app.replace("...vhost...", "?vhost=");
    if (app.indexOf("?") >= 0) {
      var params = app.substr(app.indexOf("?"));
      app = app.substr(0, app.indexOf("?"));
      if (params.indexOf("vhost=") > 0) {
        vhost = params.substr(params.indexOf("vhost=") + "vhost=".length);
        if (vhost.indexOf("&") > 0) {
          vhost = vhost.substr(0, vhost.indexOf("&"));
        }
      }
    }
    if (a.hostname == vhost) {
      var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
      if (re.test(a.hostname)) vhost = "__defaultVhost__";
    }
    var schema = "rtmp";
    if (rtmp_url.indexOf("://") > 0)
      schema = rtmp_url.substr(0, rtmp_url.indexOf("://"));
    var port = a.port;
    if (!port) {
      if (schema === "http") {
        port = 80;
      } else if (schema === "https") {
        port = 443;
      } else if (schema === "rtmp") {
        port = 1935;
      } else if (schema === "webrtc" || schema === "rtc") {
        port = 1985;
      }
    }
    var ret = {
      url: rtmp_url,
      schema: schema,
      server: a.hostname,
      port: port,
      vhost: vhost,
      app: app,
      stream: stream
    };
    JSWebrtc.FillQuery(a.search, ret);
    return ret;
  },
  HttpPost: function(url, data) {
    return new Promise(function(resolve, reject) {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (xhr.readyState === 4 && (xhr.status >= 200 && xhr.status < 300)) {
          var respone = JSON.parse(xhr.responseText);
          xhr.onreadystatechange = new Function();
          xhr = null;
          resolve(respone);
        }
      };
      xhr.open("POST", url, true);
      xhr.timeout = 5e3;
      xhr.responseType = "text";
      xhr.setRequestHeader("Content-Type", "application/json");
      xhr.send(data);
    });
  }
};
if (document.readyState === "complete") {
  JSWebrtc.CreateVideoElements();
} else {
  document.addEventListener("DOMContentLoaded", JSWebrtc.CreateVideoElements);
}
JSWebrtc.VideoElement = (function() {
  "use strict";
  var VideoElement = function(element) {
    var url = element.dataset.url;
    if (!url) {
      throw "VideoElement has no `data-url` attribute";
    }
    var addStyles = function(element, styles) {
      for (var name in styles) {
        element.style[name] = styles[name];
      }
    };
    this.container = element;
    addStyles(this.container, {
      display: "inline-block",
      position: "relative",
      minWidth: "80px",
      minHeight: "80px"
    });
    this.video = document.createElement("video");
    this.video.width = 960;
    this.video.height = 540;
    addStyles(this.video, { display: "block", width: "100%" });
    this.container.appendChild(this.video);
    this.playButton = document.createElement("div");
    this.playButton.innerHTML = VideoElement.PLAY_BUTTON;
    addStyles(this.playButton, {
      zIndex: 2,
      position: "absolute",
      top: "0",
      bottom: "0",
      left: "0",
      right: "0",
      maxWidth: "75px",
      maxHeight: "75px",
      margin: "auto",
      opacity: "0.7",
      cursor: "pointer"
    });
    this.container.appendChild(this.playButton);
    var options = { video: this.video };
    for (var option in element.dataset) {
      try {
        options[option] = JSON.parse(element.dataset[option]);
      } catch (err) {
        options[option] = element.dataset[option];
      }
    }
    // this.player = new JSWebrtc.Player(url, options);
    // element.playerInstance = this.player;
    if (options.poster && !options.autoplay) {
      options.decodeFirstFrame = false;
      this.poster = new Image();
      this.poster.src = options.poster;
      this.poster.addEventListener("load", this.posterLoaded);
      addStyles(this.poster, {
        display: "block",
        zIndex: 1,
        position: "absolute",
        top: 0,
        left: 0,
        bottom: 0,
        right: 0
      });
      this.container.appendChild(this.poster);
    }
    if (!this.player.options.streaming) {
      this.container.addEventListener("click", this.onClick.bind(this));
    }
    if (options.autoplay) {
      this.playButton.style.display = "none";
    }
    if (this.player.audioOut && !this.player.audioOut.unlocked) {
      var unlockAudioElement = this.container;
      if (options.autoplay) {
        this.unmuteButton = document.createElement("div");
        this.unmuteButton.innerHTML = VideoElement.UNMUTE_BUTTON;
        addStyles(this.unmuteButton, {
          zIndex: 2,
          position: "absolute",
          bottom: "10px",
          right: "20px",
          width: "75px",
          height: "75px",
          margin: "auto",
          opacity: "0.7",
          cursor: "pointer"
        });
        this.container.appendChild(this.unmuteButton);
        unlockAudioElement = this.unmuteButton;
      }
      this.unlockAudioBound = this.onUnlockAudio.bind(this, unlockAudioElement);
      unlockAudioElement.addEventListener(
        "touchstart",
        this.unlockAudioBound,
        false
      );
      unlockAudioElement.addEventListener("click", this.unlockAudioBound, true);
    }
  };
  VideoElement.prototype.onUnlockAudio = function(element, ev) {
    if (this.unmuteButton) {
      ev.preventDefault();
      ev.stopPropagation();
    }
    this.player.audioOut.unlock(
      function() {
        if (this.unmuteButton) {
          this.unmuteButton.style.display = "none";
        }
        element.removeEventListener("touchstart", this.unlockAudioBound);
        element.removeEventListener("click", this.unlockAudioBound);
      }.bind(this)
    );
  };
  VideoElement.prototype.onClick = function(ev) {
    if (this.player.isPlaying) {
      this.player.pause();
      this.playButton.style.display = "block";
    } else {
      this.player.play();
      this.playButton.style.display = "none";
      if (this.poster) {
        this.poster.style.display = "none";
      }
    }
  };
  VideoElement.PLAY_BUTTON =
    '<svg style="max-width: 75px; max-height: 75px;" ' +
    'viewBox="0 0 200 200" alt="Play video">' +
    '<circle cx="100" cy="100" r="90" fill="none" ' +
    'stroke-width="15" stroke="#fff"/>' +
    '<polygon points="70, 55 70, 145 145, 100" fill="#fff"/>' +
    "</svg>";
  VideoElement.UNMUTE_BUTTON =
    '<svg style="max-width: 75px; max-height: 75px;" viewBox="0 0 75 75">' +
    '<polygon class="audio-speaker" stroke="none" fill="#fff" ' +
    'points="39,13 22,28 6,28 6,47 21,47 39,62 39,13"/>' +
    '<g stroke="#fff" stroke-width="5">' +
    '<path d="M 49,50 69,26"/>' +
    '<path d="M 69,50 49,26"/>' +
    "</g>" +
    "</svg>";
  return VideoElement;
})();
JSWebrtc.Player = (function() {
  "use strict";
  var Player = function(url, options) {
    this.options = options || {};
    if (!url.match(/^webrtc?:\/\//)) {
      throw "JSWebrtc just work with webrtc";
    }
    if (!this.options.video) {
      throw "VideoElement is null";
    }
    this.urlParams = JSWebrtc.ParseUrl(url);
    this.pc = null;
    this.autoplay = !!options.autoplay || false;
    this.paused = true;
    if (this.autoplay) this.options.video.muted = true;
    this.startLoading();
  };
  Player.prototype.startLoading = function() {
    var _self = this;
    if (_self.pc) {
      _self.pc.close();
    }
    _self.pc = new RTCPeerConnection(null);
    _self.pc.ontrack = function(event) {
      _self.options.video["srcObject"] = event.streams[0];
    };
    _self.pc.addTransceiver("audio", { direction: "recvonly" });
    _self.pc.addTransceiver("video", { direction: "recvonly" });
    _self.pc
      .createOffer()
      .then(function(offer) {
        return _self.pc.setLocalDescription(offer).then(function() {
          return offer;
        });
      })
      .then(function(offer) {
        return new Promise(function(resolve, reject) {
          var port = _self.urlParams.port || 1985;
          var api = _self.urlParams.user_query.play || "/rtc/v1/play/";
          if (api.lastIndexOf("/") != api.length - 1) {
            api += "/";
          }
          var url = "http://" + _self.urlParams.server + ":" + port + api;
          let isFirst = true;
          for (var key in _self.urlParams.user_query) {
            if (key != "api" && key != "play") {
              if (isFirst) {
                url += "?" + key + "=" + _self.urlParams.user_query[key];
                isFirst = false;
              } else {
                url += "&" + key + "=" + _self.urlParams.user_query[key];
              }
            }
          }
          var data = {
            api: url,
            streamurl: _self.urlParams.url,
            clientip: null,
            sdp: offer.sdp
          };
          JSWebrtc.HttpPost(url, JSON.stringify(data)).then(
            function(res) {
              resolve(res.sdp);
            },
            function(rej) {
              reject(rej);
            }
          );
        });
      })
      .then(function(answer) {
        return _self.pc.setRemoteDescription(
          new RTCSessionDescription({ type: "answer", sdp: answer })
        );
      })
      .catch(function(reason) {
        throw reason;
      });
    if (this.autoplay) {
      this.play();
    }
  };
  Player.prototype.play = function(ev) {
    if (this.animationId) {
      return;
    }
    this.animationId = requestAnimationFrame(this.update.bind(this));
    this.paused = false;
  };
  Player.prototype.pause = function(ev) {
    if (this.paused) {
      return;
    }
    cancelAnimationFrame(this.animationId);
    this.animationId = null;
    this.isPlaying = false;
    this.paused = true;
    this.options.video.pause();
    if (this.options.onPause) {
      this.options.onPause(this);
    }
  };
  Player.prototype.stop = function(ev) {
    this.pause();
  };
  Player.prototype.destroy = function() {
    this.pause();
    this.pc && this.pc.close() && this.pc.destroy();
    this.audioOut && this.audioOut.destroy();
  };
  Player.prototype.update = function() {
    this.animationId = requestAnimationFrame(this.update.bind(this));
    if (this.options.video.readyState < 4) {
      return;
    }
    if (!this.isPlaying) {
      this.isPlaying = true;
      this.options.video.play();
      if (this.options.onPlay) {
        this.options.onPlay(this);
      }
    }
  };
  return Player;
})();