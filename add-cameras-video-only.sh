#!/bin/bash

# Camera Setup Script - Video Only Mode
# This script adds cameras with video-only streams to avoid audio authentication issues

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

print_header "Camera Setup - Video Only Mode"

# Alternative RTSP URLs for video-only streams
CAMERA1_URLS=(
    "rtsp://admin:sharpi1688@192.168.1.2:554/1/1"
    "rtsp://admin:sharpi1688@192.168.1.2:554/stream1"
    "rtsp://admin:sharpi1688@192.168.1.2:554/cam/realmonitor?channel=1&subtype=0"
    "rtsp://admin:sharpi1688@192.168.1.2:554/h264Preview_01_main"
    "rtsp://admin:sharpi1688@192.168.1.2:554/live/ch1"
    "rtsp://admin:sharpi1688@192.168.1.2:554/video1"
)

CAMERA2_URLS=(
    "rtsp://admin:sharpi1688@192.168.1.3:554/1/1"
    "rtsp://admin:sharpi1688@192.168.1.3:554/stream1"
    "rtsp://admin:sharpi1688@192.168.1.3:554/cam/realmonitor?channel=1&subtype=0"
    "rtsp://admin:sharpi1688@192.168.1.3:554/h264Preview_01_main"
    "rtsp://admin:sharpi1688@192.168.1.3:554/live/ch1"
    "rtsp://admin:sharpi1688@192.168.1.3:554/video1"
)

# Function to try adding camera with different URLs
try_camera_urls() {
    local camera_name=$1
    local stream_name=$2
    shift 2
    local urls=("$@")
    
    for url in "${urls[@]}"; do
        print_status "Trying $camera_name with URL: $url"
        
        response=$(curl -s -X POST http://localhost:80/api/v1/rtsp/play/start \
            -H "Content-Type: application/json" \
            -d "{
                \"url\": \"$url\",
                \"appName\": \"live\",
                \"streamName\": \"$stream_name\"
            }")
        
        echo "Response: $response"
        
        if echo "$response" | grep -q '"code":"200"'; then
            print_status "$camera_name added successfully with URL: $url"
            
            # Wait and check if stream is active
            sleep 3
            sources=$(curl -s http://localhost:80/api/v1/getSourceList)
            if echo "$sources" | grep -q "$stream_name"; then
                print_status "$camera_name stream is active!"
                return 0
            else
                print_status "$camera_name added but stream not active, trying next URL..."
                # Stop this attempt
                curl -s -X POST http://localhost:80/api/v1/rtsp/play/stop \
                    -H "Content-Type: application/json" \
                    -d "{\"appName\": \"live\", \"streamName\": \"$stream_name\"}" > /dev/null
            fi
        else
            print_error "Failed to add $camera_name with this URL"
        fi
        
        echo ""
    done
    
    print_error "All URLs failed for $camera_name"
    return 1
}

# Check MediaServer
if ! curl -s http://localhost:80/api/v1/version > /dev/null 2>&1; then
    print_error "MediaServer is not running. Please start it first:"
    echo "./start-mediaserver.sh"
    exit 1
fi

print_status "MediaServer is running"

print_header "Adding Camera 1"
try_camera_urls "Camera 1" "camera1" "${CAMERA1_URLS[@]}"

print_header "Adding Camera 2"
try_camera_urls "Camera 2" "camera2" "${CAMERA2_URLS[@]}"

print_header "Final Status Check"

# Check final status
print_status "Checking final stream status..."
sources=$(curl -s http://localhost:80/api/v1/getSourceList)
echo "Active sources: $sources"

rtsp_list=$(curl -s http://localhost:80/api/v1/rtsp/play/list)
echo "RTSP play list: $rtsp_list"

print_header "Stream URLs"

echo -e "${GREEN}If cameras are working, access them at:${NC}"
echo ""
echo -e "${BLUE}Local RTSP URLs:${NC}"
echo "  Camera 1: rtsp://localhost:554/live/camera1"
echo "  Camera 2: rtsp://localhost:554/live/camera2"
echo ""
echo -e "${BLUE}Local RTMP URLs:${NC}"
echo "  Camera 1: rtmp://localhost:1935/live/camera1"
echo "  Camera 2: rtmp://localhost:1935/live/camera2"
echo ""
echo -e "${BLUE}HTTP-FLV URLs:${NC}"
echo "  Camera 1: http://localhost:8080/live/camera1.flv"
echo "  Camera 2: http://localhost:8080/live/camera2.flv"
echo ""
echo -e "${BLUE}HLS URLs:${NC}"
echo "  Camera 1: http://localhost:8080/live/camera1.m3u8"
echo "  Camera 2: http://localhost:8080/live/camera2.m3u8"
echo ""
echo -e "${BLUE}Web Interface:${NC} http://localhost:3000/"

print_header "RTMP Streaming Commands"

echo -e "${YELLOW}To stream to external RTMP server:${NC}"
echo ""
echo "# Stream Camera 1 to external RTMP:"
echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
echo "  -H \"Content-Type: application/json\" \\"
echo "  -d '{"
echo "    \"url\": \"rtmp://your-server.com/live/camera1\","
echo "    \"appName\": \"live\","
echo "    \"streamName\": \"camera1\""
echo "  }'"
echo ""
echo "# Stream Camera 2 to external RTMP:"
echo "curl -X POST http://localhost:80/api/v1/rtmp/publish/start \\"
echo "  -H \"Content-Type: application/json\" \\"
echo "  -d '{"
echo "    \"url\": \"rtmp://your-server.com/live/camera2\","
echo "    \"appName\": \"live\","
echo "    \"streamName\": \"camera2\""
echo "  }'"

print_status "Camera setup completed!"
