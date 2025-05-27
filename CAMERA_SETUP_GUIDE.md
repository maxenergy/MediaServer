# Camera Setup Guide for MediaServer

## Current Status

The cameras at `192.168.1.2` and `192.168.1.3` are not reachable from the MediaServer. Here's how to troubleshoot and set them up properly.

## Troubleshooting Steps

### 1. **Check Network Connectivity**

First, verify the cameras are accessible:

```bash
# Test ping connectivity
ping 192.168.1.2
ping 192.168.1.3

# Check current network configuration
ip addr show
ip route show

# Scan for cameras on the network
nmap -sn 192.168.1.0/24  # Scan entire subnet
```

### 2. **Find Camera IP Addresses**

If the cameras have different IPs, find them:

```bash
# Scan for RTSP services on the network
nmap -p 554 192.168.1.0/24

# Or use camera manufacturer tools
# Many IP cameras have discovery tools or web interfaces
```

### 3. **Test RTSP Streams Manually**

Once you have the correct IPs, test the RTSP streams:

```bash
# Using ffprobe (if available)
ffprobe -v quiet -print_format json -show_streams "rtsp://admin:sharpi1688@192.168.1.2:554/1/1"

# Using VLC (command line)
vlc "rtsp://admin:sharpi1688@192.168.1.2:554/1/1" --intf dummy --play-and-exit

# Using ffplay
ffplay "rtsp://admin:sharpi1688@192.168.1.2:554/1/1"
```

## Manual Camera Setup

### **Option 1: Using API Commands**

Once cameras are reachable, add them manually:

```bash
# Add Camera 1
curl -X POST http://localhost:80/api/v1/rtsp/play/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "rtsp://admin:sharpi1688@192.168.1.2:554/1/1",
    "appName": "live",
    "streamName": "camera1"
  }'

# Add Camera 2
curl -X POST http://localhost:80/api/v1/rtsp/play/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "rtsp://admin:sharpi1688@192.168.1.3:554/1/1",
    "appName": "live",
    "streamName": "camera2"
  }'

# Check if cameras are added
curl -s http://localhost:80/api/v1/getSourceList

# Check RTSP play list
curl -s http://localhost:80/api/v1/rtsp/play/list
```

### **Option 2: Using Web Interface**

1. Open http://localhost:3000/ in your browser
2. Navigate to the RTSP management section
3. Add cameras using the web interface
4. Monitor stream status

## RTMP Streaming Setup

### **Stream to Local RTMP Server**

The MediaServer has a built-in RTMP server on port 1935:

```bash
# Your cameras will be available at:
# rtmp://localhost:1935/live/camera1
# rtmp://localhost:1935/live/camera2

# Test with ffplay
ffplay rtmp://localhost:1935/live/camera1
```

### **Stream to External RTMP Server**

To forward streams to an external RTMP server:

```bash
# Start RTMP publishing for Camera 1
curl -X POST http://localhost:80/api/v1/rtmp/publish/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "rtmp://your-rtmp-server.com/live/camera1",
    "appName": "live",
    "streamName": "camera1"
  }'

# Start RTMP publishing for Camera 2
curl -X POST http://localhost:80/api/v1/rtmp/publish/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "rtmp://your-rtmp-server.com/live/camera2",
    "appName": "live",
    "streamName": "camera2"
  }'
```

### **Popular RTMP Destinations**

```bash
# YouTube Live
rtmp://a.rtmp.youtube.com/live2/YOUR_STREAM_KEY

# Facebook Live
rtmp://live-api-s.facebook.com:80/rtmp/YOUR_STREAM_KEY

# Twitch
rtmp://live.twitch.tv/app/YOUR_STREAM_KEY

# Custom RTMP server
rtmp://your-server.com:1935/live/stream_name
```

## Alternative Camera URLs

Different camera brands use different RTSP URL formats:

```bash
# Common RTSP URL patterns:
rtsp://admin:password@192.168.1.2:554/1/1          # Your current format
rtsp://admin:password@192.168.1.2:554/stream1      # Alternative
rtsp://admin:password@192.168.1.2:554/cam/realmonitor?channel=1&subtype=0
rtsp://admin:password@192.168.1.2:554/h264Preview_01_main
rtsp://admin:password@192.168.1.2:554/live/ch1
rtsp://admin:password@192.168.1.2:554/video1
```

## Network Configuration

### **If Cameras Are on Different Network**

1. **Check routing**:
   ```bash
   ip route show
   traceroute 192.168.1.2
   ```

2. **Add static route** (if needed):
   ```bash
   sudo ip route add 192.168.1.0/24 via YOUR_GATEWAY_IP
   ```

3. **Configure firewall**:
   ```bash
   sudo ufw allow from 192.168.1.0/24
   sudo ufw allow out 554
   ```

## Testing and Monitoring

### **Check Stream Status**

```bash
# Get all active sources
curl -s http://localhost:80/api/v1/getSourceList | jq

# Get server information
curl -s http://localhost:80/api/v1/getServerInfo | jq

# Check RTSP play list
curl -s http://localhost:80/api/v1/rtsp/play/list | jq

# Check RTMP publish list
curl -s http://localhost:80/api/v1/rtmp/publish/list | jq
```

### **Monitor Logs**

Watch the MediaServer logs for connection status and errors.

## Quick Test with Dummy Stream

If cameras are not available, test with a dummy RTSP stream:

```bash
# Create a test RTSP stream using ffmpeg (if available)
ffmpeg -re -f lavfi -i testsrc=duration=10:size=320x240:rate=30 \
  -f rtsp rtsp://localhost:8554/test

# Then add it to MediaServer
curl -X POST http://localhost:80/api/v1/rtsp/play/start \
  -H "Content-Type: application/json" \
  -d '{
    "url": "rtsp://localhost:8554/test",
    "appName": "live",
    "streamName": "test"
  }'
```

## Summary

1. **First**: Verify camera network connectivity
2. **Second**: Test RTSP streams manually
3. **Third**: Add cameras to MediaServer
4. **Fourth**: Configure RTMP streaming
5. **Fifth**: Monitor and test streams

## Next Steps

Once cameras are working:

1. **Access streams via web interface**: http://localhost:3000/
2. **Stream to RTMP destinations** using the API
3. **Set up recording** if needed
4. **Configure transcoding** for different qualities
5. **Set up authentication** for production use

---

**Need Help?**

If cameras are still not working:
1. Check camera documentation for correct RTSP URLs
2. Use camera manufacturer's software to verify settings
3. Test with VLC or other RTSP clients first
4. Check network configuration and firewall settings
