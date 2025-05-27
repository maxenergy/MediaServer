#!/bin/bash

# Manual Camera Addition Script
# Use this script when your cameras are accessible

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

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Check if MediaServer is running
check_mediaserver() {
    if curl -s http://localhost:80/api/v1/version > /dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

# Add camera function
add_camera() {
    local camera_url=$1
    local stream_name=$2
    local camera_name=$3
    
    print_status "Adding $camera_name..."
    
    response=$(curl -s -X POST http://localhost:80/api/v1/rtsp/play/start \
        -H "Content-Type: application/json" \
        -d "{
            \"url\": \"$camera_url\",
            \"appName\": \"live\",
            \"streamName\": \"$stream_name\"
        }")
    
    echo "Response: $response"
    
    if echo "$response" | grep -q '"code":"200"'; then
        print_status "$camera_name added successfully!"
        return 0
    else
        print_error "Failed to add $camera_name"
        return 1
    fi
}

# Start RTMP publishing
start_rtmp_publish() {
    local stream_name=$1
    local rtmp_url=$2
    local camera_name=$3
    
    print_status "Starting RTMP publishing for $camera_name to $rtmp_url..."
    
    response=$(curl -s -X POST http://localhost:80/api/v1/rtmp/publish/start \
        -H "Content-Type: application/json" \
        -d "{
            \"url\": \"$rtmp_url\",
            \"appName\": \"live\",
            \"streamName\": \"$stream_name\"
        }")
    
    echo "Response: $response"
    
    if echo "$response" | grep -q '"code":"200"'; then
        print_status "$camera_name RTMP publishing started!"
        return 0
    else
        print_error "Failed to start RTMP publishing for $camera_name"
        return 1
    fi
}

print_header "Manual Camera Setup"

# Check MediaServer
if ! check_mediaserver; then
    print_error "MediaServer is not running. Please start it first:"
    echo "./start-mediaserver.sh"
    exit 1
fi

print_status "MediaServer is running"

# Camera URLs (modify these if your cameras have different IPs or credentials)
CAMERA1_URL="rtsp://admin:sharpi1688@192.168.1.2:554/1/1"
CAMERA2_URL="rtsp://admin:sharpi1688@192.168.1.3:554/1/1"

print_header "Adding Cameras"

# Add Camera 1
print_status "Attempting to add Camera 1..."
add_camera "$CAMERA1_URL" "camera1" "Camera 1"

echo ""

# Add Camera 2
print_status "Attempting to add Camera 2..."
add_camera "$CAMERA2_URL" "camera2" "Camera 2"

echo ""

# Wait for streams to establish
print_status "Waiting 5 seconds for streams to establish..."
sleep 5

print_header "Checking Stream Status"

# Check sources
print_status "Checking active sources..."
sources=$(curl -s http://localhost:80/api/v1/getSourceList)
echo "Sources: $sources"

echo ""

# Check RTSP play list
print_status "Checking RTSP play list..."
rtsp_list=$(curl -s http://localhost:80/api/v1/rtsp/play/list)
echo "RTSP Play List: $rtsp_list"

echo ""

print_header "RTMP Streaming Options"

echo -e "${BLUE}Your cameras are now available at these URLs:${NC}"
echo ""
echo -e "${GREEN}Local RTSP URLs:${NC}"
echo "  Camera 1: rtsp://localhost:554/live/camera1"
echo "  Camera 2: rtsp://localhost:554/live/camera2"
echo ""
echo -e "${GREEN}Local RTMP URLs:${NC}"
echo "  Camera 1: rtmp://localhost:1935/live/camera1"
echo "  Camera 2: rtmp://localhost:1935/live/camera2"
echo ""
echo -e "${GREEN}HTTP-FLV URLs:${NC}"
echo "  Camera 1: http://localhost:8080/live/camera1.flv"
echo "  Camera 2: http://localhost:8080/live/camera2.flv"
echo ""
echo -e "${GREEN}HLS URLs:${NC}"
echo "  Camera 1: http://localhost:8080/live/camera1.m3u8"
echo "  Camera 2: http://localhost:8080/live/camera2.m3u8"

echo ""
echo -e "${YELLOW}To stream to external RTMP server:${NC}"
echo ""
echo "# For Camera 1:"
echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
echo "  -H \"Content-Type: application/json\" \\"
echo "  -d '{"
echo "    \"url\": \"rtmp://your-server.com/live/camera1\","
echo "    \"appName\": \"live\","
echo "    \"streamName\": \"camera1\""
echo "  }'"
echo ""
echo "# For Camera 2:"
echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
echo "  -H \"Content-Type: application/json\" \\"
echo "  -d '{"
echo "    \"url\": \"rtmp://your-server.com/live/camera2\","
echo "    \"appName\": \"live\","
echo "    \"streamName\": \"camera2\""
echo "  }'"

echo ""
echo -e "${BLUE}Web Interface:${NC} http://localhost:3000/"

print_header "Setup Complete"
print_status "Camera setup script finished!"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Open http://localhost:3000/ to manage streams via web interface"
echo "2. Test streams with VLC or other media players"
echo "3. Configure RTMP publishing to external servers if needed"
echo "4. Set up recording or transcoding as required"
