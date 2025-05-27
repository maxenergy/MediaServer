#!/bin/bash

# Camera Setup Script for MediaServer
# This script helps configure RTSP cameras and stream them to RTMP

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Camera configurations
CAMERA1_IP="192.168.1.2"
CAMERA2_IP="192.168.1.3"
CAMERA_USER="admin"
CAMERA_PASS="sharpi1688"
CAMERA_PORT="554"
CAMERA_PATH="/1/1"

CAMERA1_URL="rtsp://${CAMERA_USER}:${CAMERA_PASS}@${CAMERA1_IP}:${CAMERA_PORT}${CAMERA_PATH}"
CAMERA2_URL="rtsp://${CAMERA_USER}:${CAMERA_PASS}@${CAMERA2_IP}:${CAMERA_PORT}${CAMERA_PATH}"

print_header "Camera Setup and RTMP Streaming"

# Function to test camera connectivity
test_camera() {
    local camera_ip=$1
    local camera_name=$2
    
    print_status "Testing connectivity to $camera_name ($camera_ip)..."
    
    if ping -c 1 -W 3 $camera_ip > /dev/null 2>&1; then
        print_status "$camera_name is reachable"
        return 0
    else
        print_error "$camera_name is not reachable"
        return 1
    fi
}

# Function to test RTSP stream with ffprobe
test_rtsp_stream() {
    local url=$1
    local camera_name=$2
    
    print_status "Testing RTSP stream for $camera_name..."
    
    if command -v ffprobe &> /dev/null; then
        if timeout 10 ffprobe -v quiet -print_format json -show_streams "$url" > /dev/null 2>&1; then
            print_status "$camera_name RTSP stream is accessible"
            return 0
        else
            print_error "$camera_name RTSP stream is not accessible"
            return 1
        fi
    else
        print_warning "ffprobe not found, skipping stream test"
        return 0
    fi
}

# Function to add camera to MediaServer
add_camera() {
    local url=$1
    local stream_name=$2
    local camera_name=$3
    
    print_status "Adding $camera_name to MediaServer..."
    
    response=$(curl -s -X POST http://localhost:80/api/v1/rtsp/play/start \
        -H "Content-Type: application/json" \
        -d "{
            \"url\": \"$url\",
            \"appName\": \"live\",
            \"streamName\": \"$stream_name\"
        }")
    
    if echo "$response" | grep -q '"code":"200"'; then
        print_status "$camera_name added successfully"
        return 0
    else
        print_error "Failed to add $camera_name: $response"
        return 1
    fi
}

# Function to start RTMP publishing
start_rtmp_publish() {
    local stream_name=$1
    local rtmp_url=$2
    local camera_name=$3
    
    print_status "Starting RTMP publish for $camera_name..."
    
    response=$(curl -s -X POST http://localhost:80/api/v1/rtmp/publish/start \
        -H "Content-Type: application/json" \
        -d "{
            \"url\": \"$rtmp_url\",
            \"appName\": \"live\",
            \"streamName\": \"$stream_name\"
        }")
    
    if echo "$response" | grep -q '"code":"200"'; then
        print_status "$camera_name RTMP publishing started"
        return 0
    else
        print_error "Failed to start RTMP publishing for $camera_name: $response"
        return 1
    fi
}

# Function to check MediaServer status
check_mediaserver() {
    print_status "Checking MediaServer status..."
    
    if curl -s http://localhost:80/api/v1/version > /dev/null 2>&1; then
        print_status "MediaServer is running"
        return 0
    else
        print_error "MediaServer is not running. Please start it first with: ./start-mediaserver.sh"
        return 1
    fi
}

# Main execution
print_header "Step 1: Check MediaServer"
if ! check_mediaserver; then
    exit 1
fi

print_header "Step 2: Test Camera Connectivity"
camera1_reachable=false
camera2_reachable=false

if test_camera $CAMERA1_IP "Camera 1"; then
    camera1_reachable=true
fi

if test_camera $CAMERA2_IP "Camera 2"; then
    camera2_reachable=true
fi

print_header "Step 3: Test RTSP Streams"
camera1_stream_ok=false
camera2_stream_ok=false

if [ "$camera1_reachable" = true ]; then
    if test_rtsp_stream "$CAMERA1_URL" "Camera 1"; then
        camera1_stream_ok=true
    fi
fi

if [ "$camera2_reachable" = true ]; then
    if test_rtsp_stream "$CAMERA2_URL" "Camera 2"; then
        camera2_stream_ok=true
    fi
fi

print_header "Step 4: Add Cameras to MediaServer"

if [ "$camera1_stream_ok" = true ]; then
    add_camera "$CAMERA1_URL" "camera1" "Camera 1"
else
    print_warning "Skipping Camera 1 due to connectivity issues"
fi

if [ "$camera2_stream_ok" = true ]; then
    add_camera "$CAMERA2_URL" "camera2" "Camera 2"
else
    print_warning "Skipping Camera 2 due to connectivity issues"
fi

# Wait a moment for streams to establish
sleep 3

print_header "Step 5: Check Stream Status"
print_status "Checking active sources..."
sources=$(curl -s http://localhost:80/api/v1/getSourceList)
echo "Active sources: $sources"

print_header "Step 6: RTMP Streaming Options"
echo -e "${BLUE}You can now stream to RTMP using these URLs:${NC}"
echo ""
echo -e "${GREEN}Camera 1 RTMP URLs:${NC}"
echo "  - Local RTMP: rtmp://localhost:1935/live/camera1"
echo "  - External RTMP: rtmp://YOUR_RTMP_SERVER/live/camera1"
echo ""
echo -e "${GREEN}Camera 2 RTMP URLs:${NC}"
echo "  - Local RTMP: rtmp://localhost:1935/live/camera2"
echo "  - External RTMP: rtmp://YOUR_RTMP_SERVER/live/camera2"
echo ""
echo -e "${YELLOW}To stream to external RTMP server:${NC}"
echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
echo "  -H \"Content-Type: application/json\" \\"
echo "  -d '{"
echo "    \"url\": \"rtmp://your-server.com/live/camera1\","
echo "    \"appName\": \"live\","
echo "    \"streamName\": \"camera1\""
echo "  }'"

print_header "Setup Complete"

if [ "$camera1_stream_ok" = true ] || [ "$camera2_stream_ok" = true ]; then
    print_status "Camera setup completed successfully!"
    echo ""
    echo -e "${GREEN}Access your streams:${NC}"
    echo "  - Web interface: http://localhost:3000/"
    echo "  - RTSP playback: rtsp://localhost:554/live/camera1"
    echo "  - RTSP playback: rtsp://localhost:554/live/camera2"
    echo "  - RTMP playback: rtmp://localhost:1935/live/camera1"
    echo "  - RTMP playback: rtmp://localhost:1935/live/camera2"
else
    print_error "No cameras were successfully configured"
    echo ""
    echo -e "${YELLOW}Troubleshooting steps:${NC}"
    echo "1. Check camera IP addresses and network connectivity"
    echo "2. Verify camera credentials (username/password)"
    echo "3. Test RTSP URLs manually with VLC or ffplay"
    echo "4. Check camera RTSP settings and port configuration"
fi
