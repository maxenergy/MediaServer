# MediaServer Testing Guide

This guide provides instructions for testing the synchronized C++ backend and JavaScript frontend APIs.

## Prerequisites

1. **C++ Backend**: Built and ready in `build/SimpleMediaServer`
2. **Node.js**: Version 16+ installed
3. **npm**: Package manager for frontend dependencies

## Quick Start

### Option 1: Use the Startup Script (Recommended)

```bash
# Make sure you're in the MediaServer root directory
./start-mediaserver.sh
```

This script will:
- Start the C++ backend on port 80 (API) and other protocol ports
- Install frontend dependencies if needed
- Start the Vue.js frontend on port 3000
- Display all service URLs

### Option 2: Manual Startup

#### Start Backend
```bash
cd build
./SimpleMediaServer
```

#### Start Frontend (in another terminal)
```bash
cd web-admin
npm install  # Only needed first time
npm run dev
```

## Service URLs

Once both services are running:

- **Frontend Web Interface**: http://localhost:3000/
- **Backend API Base**: http://localhost:80/api/v1/
- **RTSP Server**: rtsp://localhost:554/
- **RTMP Server**: rtmp://localhost:1935/
- **WebRTC Endpoints**: http://localhost:80/api/v1/rtc/
- **SRT Server**: srt://localhost:6666/

## API Testing

### 1. Basic Server Information

Test the basic server APIs:

```bash
# Get server version
curl http://localhost:80/api/v1/version

# Get server info
curl http://localhost:80/api/v1/getServerInfo

# Get configuration
curl http://localhost:80/api/v1/config
```

### 2. Stream Management

```bash
# Get source list
curl http://localhost:80/api/v1/getSourceList

# Get client list
curl http://localhost:80/api/v1/getClientList
```

### 3. RTSP API Testing

```bash
# Start RTSP playback
curl -X POST http://localhost:80/api/v1/rtsp/play/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "rtsp://example.com/stream",
    "appName": "live",
    "streamName": "test"
  }'

# Get RTSP play list
curl http://localhost:80/api/v1/rtsp/play/list
```

### 4. WebRTC API Testing

```bash
# WebRTC play request
curl -X POST http://localhost:80/api/v1/rtc/play \
  -H "Content-Type: application/json" \
  -d '{
    "appName": "live",
    "streamName": "test",
    "enableDtls": 1,
    "sdp": "v=0..."
  }'
```

### 5. New RTP API Testing

```bash
# Create RTP receiver
curl -X POST http://localhost:80/api/v1/rtp/recv/create \
  -H "Content-Type: application/json" \
  -d '{
    "active": 1,
    "ssrc": 12345,
    "streamName": "test",
    "appName": "live"
  }'

# Create RTP sender
curl -X POST http://localhost:80/api/v1/rtp/send/create \
  -H "Content-Type: application/json" \
  -d '{
    "active": 1,
    "ssrc": 12345,
    "port": 5004,
    "socketType": 2,
    "streamName": "test",
    "appName": "live",
    "ip": "127.0.0.1"
  }'
```

### 6. HTTP Stream API Testing

```bash
# Start FLV playback
curl -X POST http://localhost:80/api/v1/http/flv/play/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "http://example.com/stream.flv",
    "appName": "live",
    "streamName": "test"
  }'

# Start HLS playback
curl -X POST http://localhost:80/api/v1/http/hls/play/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "http://example.com/playlist.m3u8",
    "appName": "live",
    "streamName": "test"
  }'
```

### 7. FFmpeg API Testing

```bash
# Add transcoding task
curl -X POST http://localhost:80/api/v1/ffmpeg/task/add \
  -H "Content-Type: application/json" \
  -d '{
    "path": "/live/test",
    "videoCodec": "h264",
    "audioCodec": "aac"
  }'
```

## Frontend Testing

### 1. Access Web Interface

Open http://localhost:3000/ in your browser to access the management interface.

### 2. Test API Integration

The frontend should be able to:
- Display server information
- Show stream lists
- Manage RTSP/RTMP streams
- Control WebRTC sessions
- Monitor system status

### 3. Browser Console Testing

Open browser developer tools and test APIs directly:

```javascript
// Test the new RTP API
import { rtpAPI } from '@/api'

// Create RTP receiver
rtpAPI.createReceiver({
  active: 1,
  ssrc: 12345,
  streamName: 'test',
  appName: 'live'
}).then(response => {
  console.log('RTP receiver created:', response)
})

// Test HTTP Stream API
import { httpStreamAPI } from '@/api'

// Start FLV playback
httpStreamAPI.flv.startPlay({
  url: 'http://example.com/stream.flv',
  appName: 'live',
  streamName: 'test'
}).then(response => {
  console.log('FLV playback started:', response)
})
```

## Troubleshooting

### Backend Issues

1. **Config file not found**: Make sure `server.json` is in the build directory
2. **Port conflicts**: Check if ports 80, 554, 1935, etc. are available
3. **SSL certificates**: Copy SSL files to build directory for HTTPS

### Frontend Issues

1. **Dependencies**: Run `npm install` in web-admin directory
2. **API proxy errors**: Check if backend is running on port 80
3. **CORS issues**: Backend should allow frontend origin

### Common Solutions

```bash
# Check if ports are in use
netstat -tulpn | grep :80
netstat -tulpn | grep :3000

# Kill processes using ports
sudo fuser -k 80/tcp
sudo fuser -k 3000/tcp

# Restart with clean state
./start-mediaserver.sh
```

## Testing Checklist

- [ ] Backend starts without errors
- [ ] Frontend starts and loads at http://localhost:3000/
- [ ] Basic API endpoints respond (version, config, server info)
- [ ] Stream management APIs work
- [ ] Protocol-specific APIs (RTSP, RTMP, WebRTC) respond
- [ ] New APIs (RTP, HTTP Stream, FFmpeg) are accessible
- [ ] Frontend can communicate with backend APIs
- [ ] No CORS or proxy errors in browser console

## Performance Testing

For load testing, you can use tools like:

```bash
# Test API performance
ab -n 1000 -c 10 http://localhost:80/api/v1/version

# Test with curl in loop
for i in {1..100}; do
  curl -s http://localhost:80/api/v1/getServerInfo > /dev/null
  echo "Request $i completed"
done
```

## Next Steps

1. **Stream Testing**: Set up actual RTSP/RTMP streams for end-to-end testing
2. **WebRTC Testing**: Use WebRTC test pages to verify real-time communication
3. **Load Testing**: Test with multiple concurrent streams
4. **Integration Testing**: Test all protocol combinations
5. **UI Testing**: Verify all frontend features work with real data

---

**Note**: This testing setup provides a complete development environment for the MediaServer with all synchronized APIs ready for testing and development.
