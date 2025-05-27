# MediaServer API Documentation

This document describes all available API endpoints synchronized from the C++ backend to the frontend.

## Base Configuration

All APIs use the base URL `/api/v1` and expect JSON content type.

## API Modules

### 1. Configuration API (`configAPI`)

- **GET /config** - Get server configuration
- **POST /config** - Update server configuration
  ```javascript
  configAPI.updateConfig({
    Key1: "section",
    Key2: "subsection", 
    Key3: "key",
    Key4: "subkey",
    Value: "new_value"
  })
  ```

### 2. Stream Management API (`streamAPI`)

- **GET /getSourceList** - Get list of all media sources
- **GET /getSourceInfo** - Get specific source information
- **POST /closeSource** - Close a media source
- **GET /getClientList** - Get list of connected clients
- **POST /closeClient** - Close a client connection
- **GET /streams/keyframe** - Get keyframe information
- **POST /streams/setStampMode** - Set timestamp mode

### 3. Server API (`serverAPI`)

- **GET /getServerInfo** - Get server information
- **GET /version** - Get server version
- **GET /getLoopList** - Get event loop list
- **POST /exitServer** - Exit server

### 4. RTSP API (`rtspAPI`)

- **POST /rtsp/streams/create** - Create RTSP stream
- **POST /rtsp/play/start** - Start RTSP playback
- **POST /rtsp/play/stop** - Stop RTSP playback
- **GET /rtsp/play/list** - Get RTSP playback list
- **POST /rtsp/publish/start** - Start RTSP publishing
- **POST /rtsp/publish/stop** - Stop RTSP publishing
- **GET /rtsp/publish/list** - Get RTSP publishing list
- **POST /rtsp/server/create** - Create RTSP server
- **POST /rtsp/server/stop** - Stop RTSP server

### 5. RTMP API (`rtmpAPI`)

- **POST /rtmp/create** - Create RTMP stream
- **POST /rtmp/play/start** - Start RTMP playback
- **POST /rtmp/play/stop** - Stop RTMP playback
- **GET /rtmp/play/list** - Get RTMP playback list
- **POST /rtmp/publish/start** - Start RTMP publishing
- **POST /rtmp/publish/stop** - Stop RTMP publishing
- **GET /rtmp/publish/list** - Get RTMP publishing list

### 6. WebRTC API (`webrtcAPI`)

- **POST /rtc/play** - WebRTC playback
- **POST /rtc/publish** - WebRTC publishing
- **POST /rtc/whep** - WHEP (WebRTC-HTTP Egress Protocol) playback
- **POST /rtc/whip** - WHIP (WebRTC-HTTP Ingress Protocol) publishing
- **POST /rtc/pull/start** - Start WebRTC pull
- **POST /rtc/pull/stop** - Stop WebRTC pull
- **GET /rtc/pull/list** - Get WebRTC pull list
- **POST /rtc/push/start** - Start WebRTC push
- **POST /rtc/push/stop** - Stop WebRTC push
- **GET /rtc/push/list** - Get WebRTC push list

### 7. SRT API (`srtAPI`)

- **POST /srt/pull/start** - Start SRT pull
- **POST /srt/pull/stop** - Stop SRT pull
- **GET /srt/pull/list** - Get SRT pull list
- **POST /srt/push/start** - Start SRT push
- **POST /srt/push/stop** - Stop SRT push
- **GET /srt/push/list** - Get SRT push list

### 8. GB28181 API (`gb28181API`)

- **POST /gb28181/recv/create** - Create GB28181 receiver
- **POST /gb28181/send/create** - Create GB28181 sender
- **POST /gb28181/recv/stop** - Stop GB28181 receiver
- **POST /gb28181/send/stop** - Stop GB28181 sender

### 9. JT1078 API (`jt1078API`)

- **POST /jt1078/create** - Create JT1078 instance
- **POST /jt1078/delete** - Delete JT1078 instance
- **POST /jt1078/server/open** - Open JT1078 server
- **POST /jt1078/server/close** - Close JT1078 server
- **POST /jt1078/talk/start** - Start JT1078 talk
- **POST /jt1078/talk/stop** - Stop JT1078 talk
- **POST /jt1078/send/start** - Start JT1078 send
- **POST /jt1078/send/stop** - Stop JT1078 send
- **GET /jt1078/port/info** - Get JT1078 port information

### 10. Record API (`recordAPI`)

- **POST /record/start** - Start recording
- **POST /record/stop** - Stop recording
- **GET /record/list** - Get recording list

### 11. VOD API (`vodAPI`)

- **POST /vod/start** - Start VOD playback
- **POST /vod/stop** - Stop VOD playback
- **POST /vod/control** - Control VOD playback

## New API Modules Added

### 12. RTP API (`rtpAPI`)

Real-time Transport Protocol management:
- **POST /rtp/recv/create** - Create RTP receiver
- **POST /rtp/send/create** - Create RTP sender
- **POST /rtp/send/start** - Start RTP sender
- **POST /rtp/recv/stop** - Stop RTP receiver
- **POST /rtp/send/stop** - Stop RTP sender

### 13. HTTP Stream API (`httpStreamAPI`)

HTTP-based streaming protocols:

#### FLV Streaming
- **POST /http/flv/play/start** - Start FLV playback
- **POST /http/flv/play/stop** - Stop FLV playback
- **GET /http/flv/play/list** - Get FLV playback list

#### HLS Streaming
- **POST /http/hls/play/start** - Start HLS playback
- **POST /http/hls/play/stop** - Stop HLS playback
- **GET /http/hls/play/list** - Get HLS playback list

#### PS VOD Streaming
- **POST /http/ps/vod/play/start** - Start PS VOD playback
- **POST /http/ps/vod/play/stop** - Stop PS VOD playback
- **GET /http/ps/vod/play/list** - Get PS VOD playback list

### 14. FFmpeg API (`ffmpegAPI`)

Transcoding task management:
- **POST /ffmpeg/task/add** - Add transcoding task
- **POST /ffmpeg/task/del** - Delete transcoding task
- **POST /ffmpeg/task/reconfig** - Reconfigure transcoding task

### 15. Hook API (`hookAPI`)

Event hooks for stream lifecycle:
- **POST /onStreamStatus** - Stream status change hook
- **POST /onPublish** - Publish authentication hook
- **POST /onPlay** - Play authentication hook
- **POST /onNonePlayer** - No player hook
- **POST /onKeepAlive** - Keep alive hook

### 16. Test API (`testAPI`)

Testing and debugging:
- **POST /test/work/task/add** - Add work task
- **POST /test/work/ordertask/add** - Add ordered work task

### 17. WebSocket API (`websocketAPI`)

WebSocket streaming:
- **POST /websocket/publish/start** - Start WebSocket publishing

## Usage Examples

```javascript
import { rtspAPI, webrtcAPI, rtpAPI } from '@/api'

// Start RTSP playback
await rtspAPI.startPlay({
  url: 'rtsp://example.com/stream',
  appName: 'live',
  streamName: 'test'
})

// Create RTP receiver
await rtpAPI.createReceiver({
  active: 1,
  ssrc: 12345,
  streamName: 'test',
  appName: 'live'
})

// Start WebRTC playback
await webrtcAPI.play({
  appName: 'live',
  streamName: 'test',
  enableDtls: 1,
  sdp: 'v=0...'
})
```

## Error Handling

All APIs return responses in the format:
```json
{
  "code": "200",
  "msg": "success",
  "data": {}
}
```

Error responses include appropriate error codes and messages.
