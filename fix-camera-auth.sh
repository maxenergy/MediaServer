#!/bin/bash

# Camera Authentication Fix Script
# This script tries different RTSP URL formats and authentication methods

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_header "Camera Authentication Fix"

# Camera configurations
CAMERA1_IP="192.168.1.2"
CAMERA2_IP="192.168.1.3"
USER="admin"
PASS="sharpi1688"
PORT="554"

# Different RTSP URL patterns to try
declare -a URL_PATTERNS=(
    "/1/1"
    "/stream1"
    "/cam/realmonitor?channel=1&subtype=0"
    "/h264Preview_01_main"
    "/live/ch1"
    "/video1"
    "/1"
    "/ch1"
    "/stream"
    "/live"
    "/main"
    "/cam1"
    "/channel1"
    "/1/main"
    "/1/sub"
    "/onvif1"
    "/axis-media/media.amp"
    "/MediaInput/h264"
    "/streaming/channels/1"
    "/streaming/channels/101"
    "/ISAPI/Streaming/channels/1"
    "/ISAPI/Streaming/channels/101"
)

# Function to test RTSP URL
test_rtsp_url() {
    local ip=$1
    local pattern=$2
    local camera_name=$3
    local stream_name=$4
    
    local url="rtsp://${USER}:${PASS}@${ip}:${PORT}${pattern}"
    
    print_status "Testing $camera_name with URL: $url"
    
    # Add camera to MediaServer
    response=$(curl -s -X POST http://localhost:80/api/v1/rtsp/play/start \
        -H "Content-Type: application/json" \
        -d "{
            \"url\": \"$url\",
            \"appName\": \"live\",
            \"streamName\": \"$stream_name\"
        }")
    
    if echo "$response" | grep -q '"code":"200"'; then
        print_status "API call successful, waiting for stream to establish..."
        
        # Wait for stream to establish
        sleep 5
        
        # Check if stream is active
        sources=$(curl -s http://localhost:80/api/v1/getSourceList)
        if echo "$sources" | grep -q "$stream_name"; then
            print_status "✅ SUCCESS! $camera_name is working with URL: $url"
            echo ""
            echo -e "${GREEN}Stream URLs for $camera_name:${NC}"
            echo "  RTSP: rtsp://localhost:554/live/$stream_name"
            echo "  RTMP: rtmp://localhost:1935/live/$stream_name"
            echo "  HTTP-FLV: http://localhost:8080/live/$stream_name.flv"
            echo "  HLS: http://localhost:8080/live/$stream_name.m3u8"
            echo ""
            return 0
        else
            print_warning "Stream added but not active, trying next URL..."
            # Stop this attempt
            curl -s -X POST http://localhost:80/api/v1/rtsp/play/stop \
                -H "Content-Type: application/json" \
                -d "{\"appName\": \"live\", \"streamName\": \"$stream_name\"}" > /dev/null
        fi
    else
        print_error "API call failed: $response"
    fi
    
    return 1
}

# Function to try all URL patterns for a camera
try_camera() {
    local ip=$1
    local camera_name=$2
    local stream_name=$3
    
    print_header "Testing $camera_name ($ip)"
    
    for pattern in "${URL_PATTERNS[@]}"; do
        if test_rtsp_url "$ip" "$pattern" "$camera_name" "$stream_name"; then
            return 0
        fi
        sleep 2
    done
    
    print_error "❌ All URL patterns failed for $camera_name"
    return 1
}

# Check MediaServer
if ! curl -s http://localhost:80/api/v1/version > /dev/null 2>&1; then
    print_error "MediaServer is not running. Please start it first:"
    echo "./start-mediaserver.sh"
    exit 1
fi

print_status "MediaServer is running"

# Test cameras
camera1_success=false
camera2_success=false

if try_camera "$CAMERA1_IP" "Camera 1" "camera1"; then
    camera1_success=true
fi

if try_camera "$CAMERA2_IP" "Camera 2" "camera2"; then
    camera2_success=true
fi

print_header "Final Results"

if [ "$camera1_success" = true ] || [ "$camera2_success" = true ]; then
    print_status "✅ At least one camera is working!"
    
    echo ""
    echo -e "${GREEN}Working cameras will now appear in:${NC}"
    echo "  📱 Web Interface: http://localhost:3000/"
    echo "  📋 Streams List: http://localhost:3000/#/streams/list"
    echo "  🎛️  RTSP Management: http://localhost:3000/#/protocols/rtsp"
    echo ""
    
    if [ "$camera1_success" = true ] && [ "$camera2_success" = true ]; then
        print_status "🎉 Both cameras are working!"
    elif [ "$camera1_success" = true ]; then
        print_status "✅ Camera 1 is working"
        print_warning "❌ Camera 2 failed - check IP address and credentials"
    else
        print_status "✅ Camera 2 is working"
        print_warning "❌ Camera 1 failed - check IP address and credentials"
    fi
    
    echo ""
    echo -e "${BLUE}To stream to external RTMP server:${NC}"
    if [ "$camera1_success" = true ]; then
        echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
        echo "  -H \"Content-Type: application/json\" \\"
        echo "  -d '{\"url\": \"rtmp://your-server.com/live/camera1\", \"appName\": \"live\", \"streamName\": \"camera1\"}'"
    fi
    if [ "$camera2_success" = true ]; then
        echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
        echo "  -H \"Content-Type: application/json\" \\"
        echo "  -d '{\"url\": \"rtmp://your-server.com/live/camera2\", \"appName\": \"live\", \"streamName\": \"camera2\"}'"
    fi
    
else
    print_error "❌ Both cameras failed to connect"
    echo ""
    echo -e "${YELLOW}Troubleshooting suggestions:${NC}"
    echo "1. Check camera IP addresses:"
    echo "   ping $CAMERA1_IP"
    echo "   ping $CAMERA2_IP"
    echo ""
    echo "2. Verify camera credentials:"
    echo "   Username: $USER"
    echo "   Password: $PASS"
    echo ""
    echo "3. Test with VLC or other RTSP client:"
    echo "   vlc rtsp://$USER:$PASS@$CAMERA1_IP:$PORT/1/1"
    echo ""
    echo "4. Check camera web interface:"
    echo "   http://$CAMERA1_IP"
    echo "   http://$CAMERA2_IP"
    echo ""
    echo "5. Try different credentials or check camera manual for correct RTSP URL format"
fi

print_header "Setup Complete"
