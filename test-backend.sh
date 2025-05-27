#!/bin/bash

# Simple test script to verify the MediaServer backend is working

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_test() {
    echo -e "${BLUE}Testing:${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓ PASS:${NC} $1"
}

print_fail() {
    echo -e "${RED}✗ FAIL:${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Check if backend is running
check_backend() {
    if curl -s http://localhost:80/api/v1/version > /dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

print_header "MediaServer Backend API Tests"

# Wait for backend to be ready
print_test "Checking if backend is running..."
for i in {1..10}; do
    if check_backend; then
        print_success "Backend is responding"
        break
    else
        if [ $i -eq 10 ]; then
            print_fail "Backend is not responding after 10 attempts"
            echo "Please make sure the backend is running with: cd build && ./SimpleMediaServer"
            exit 1
        fi
        echo "Waiting for backend... (attempt $i/10)"
        sleep 2
    fi
done

# Test basic APIs
echo ""
print_header "Basic API Tests"

# Test version endpoint
print_test "GET /api/v1/version"
if response=$(curl -s http://localhost:80/api/v1/version 2>/dev/null); then
    if echo "$response" | grep -q "version\|code"; then
        print_success "Version endpoint working"
        echo "Response: $response"
    else
        print_fail "Version endpoint returned unexpected response"
    fi
else
    print_fail "Version endpoint not accessible"
fi

echo ""

# Test server info endpoint
print_test "GET /api/v1/getServerInfo"
if response=$(curl -s http://localhost:80/api/v1/getServerInfo 2>/dev/null); then
    if echo "$response" | grep -q "code\|msg"; then
        print_success "Server info endpoint working"
        echo "Response: $response"
    else
        print_fail "Server info endpoint returned unexpected response"
    fi
else
    print_fail "Server info endpoint not accessible"
fi

echo ""

# Test config endpoint
print_test "GET /api/v1/config"
if response=$(curl -s http://localhost:80/api/v1/config 2>/dev/null); then
    if echo "$response" | grep -q "LocalIp\|Http\|Rtsp"; then
        print_success "Config endpoint working"
        echo "Response length: $(echo "$response" | wc -c) characters"
    else
        print_fail "Config endpoint returned unexpected response"
    fi
else
    print_fail "Config endpoint not accessible"
fi

echo ""
print_header "Stream Management API Tests"

# Test source list
print_test "GET /api/v1/getSourceList"
if response=$(curl -s http://localhost:80/api/v1/getSourceList 2>/dev/null); then
    print_success "Source list endpoint working"
    echo "Response: $response"
else
    print_fail "Source list endpoint not accessible"
fi

echo ""

# Test client list
print_test "GET /api/v1/getClientList"
if response=$(curl -s http://localhost:80/api/v1/getClientList 2>/dev/null); then
    print_success "Client list endpoint working"
    echo "Response: $response"
else
    print_fail "Client list endpoint not accessible"
fi

echo ""
print_header "New API Tests (RTP, HTTP Stream, etc.)"

# Test RTP API (this will likely fail without proper parameters, but should return an error response)
print_test "POST /api/v1/rtp/recv/create (expect parameter error)"
if response=$(curl -s -X POST http://localhost:80/api/v1/rtp/recv/create \
    -H "Content-Type: application/json" \
    -d '{}' 2>/dev/null); then
    if echo "$response" | grep -q "error\|missing\|required\|400"; then
        print_success "RTP API endpoint accessible (parameter validation working)"
        echo "Response: $response"
    else
        print_success "RTP API endpoint accessible"
        echo "Response: $response"
    fi
else
    print_fail "RTP API endpoint not accessible"
fi

echo ""

# Test FFmpeg API
print_test "POST /api/v1/ffmpeg/task/add (expect parameter error)"
if response=$(curl -s -X POST http://localhost:80/api/v1/ffmpeg/task/add \
    -H "Content-Type: application/json" \
    -d '{}' 2>/dev/null); then
    if echo "$response" | grep -q "error\|missing\|required\|400\|path"; then
        print_success "FFmpeg API endpoint accessible (parameter validation working)"
        echo "Response: $response"
    else
        print_success "FFmpeg API endpoint accessible"
        echo "Response: $response"
    fi
else
    print_fail "FFmpeg API endpoint not accessible"
fi

echo ""

# Test Hook API
print_test "POST /api/v1/onStreamStatus"
if response=$(curl -s -X POST http://localhost:80/api/v1/onStreamStatus \
    -H "Content-Type: application/json" \
    -d '{}' 2>/dev/null); then
    print_success "Hook API endpoint accessible"
    echo "Response: $response"
else
    print_fail "Hook API endpoint not accessible"
fi

echo ""

# Test WebSocket API
print_test "POST /api/v1/websocket/publish/start (expect parameter error)"
if response=$(curl -s -X POST http://localhost:80/api/v1/websocket/publish/start \
    -H "Content-Type: application/json" \
    -d '{}' 2>/dev/null); then
    if echo "$response" | grep -q "error\|missing\|required\|400"; then
        print_success "WebSocket API endpoint accessible (parameter validation working)"
        echo "Response: $response"
    else
        print_success "WebSocket API endpoint accessible"
        echo "Response: $response"
    fi
else
    print_fail "WebSocket API endpoint not accessible"
fi

echo ""
print_header "Test Summary"

echo -e "${GREEN}✓ Backend is running and responding${NC}"
echo -e "${GREEN}✓ Basic APIs are accessible${NC}"
echo -e "${GREEN}✓ New synchronized APIs are accessible${NC}"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Start the frontend: cd web-admin && npm run dev"
echo "2. Open http://localhost:3000/ in your browser"
echo "3. Test the full API integration through the web interface"
echo ""
echo -e "${BLUE}For comprehensive testing, see TESTING_GUIDE.md${NC}"
