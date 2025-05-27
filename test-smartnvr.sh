#!/bin/bash

# SmartNVR Test Script
# Tests the AI API endpoints and basic functionality

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Server configuration
SERVER_HOST="localhost"
SERVER_PORT="80"
BASE_URL="http://${SERVER_HOST}:${SERVER_PORT}"

# Function to print colored output
print_test() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

print_fail() {
    echo -e "${RED}[FAIL]${NC} $1"
}

print_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

# Function to test API endpoint
test_api() {
    local method=$1
    local endpoint=$2
    local data=$3
    local expected_code=$4
    
    print_test "Testing ${method} ${endpoint}"
    
    if [[ "$method" == "GET" ]]; then
        response=$(curl -s -w "\n%{http_code}" "${BASE_URL}${endpoint}")
    else
        response=$(curl -s -w "\n%{http_code}" -X "${method}" \
            -H "Content-Type: application/json" \
            -d "${data}" \
            "${BASE_URL}${endpoint}")
    fi
    
    # Extract HTTP code and body
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n -1)
    
    if [[ "$http_code" == "$expected_code" ]]; then
        print_success "HTTP $http_code - $endpoint"
        if [[ -n "$body" ]]; then
            echo "Response: $body" | head -c 200
            echo "..."
        fi
    else
        print_fail "HTTP $http_code (expected $expected_code) - $endpoint"
        echo "Response: $body"
        return 1
    fi
    
    echo ""
}

# Check if server is running
check_server() {
    print_info "Checking if SmartNVR server is running..."
    
    if curl -s "${BASE_URL}/api/v1/getServerInfo" > /dev/null; then
        print_success "Server is running at ${BASE_URL}"
    else
        print_fail "Server is not running. Please start it first:"
        echo "  cd build && ./SimpleMediaServer"
        exit 1
    fi
    echo ""
}

# Test basic server info
test_server_info() {
    print_info "Testing basic server functionality..."
    test_api "GET" "/api/v1/getServerInfo" "" "200"
}

# Test AI endpoints (these will work even without OpenCV if AI is disabled)
test_ai_endpoints() {
    print_info "Testing AI API endpoints..."
    
    # Test AI stats
    test_api "GET" "/api/v1/ai/stats" "" "200"
    
    # Test AI detection status
    test_api "GET" "/api/v1/ai/detection/status" "" "200"
    
    # Test AI config
    test_api "GET" "/api/v1/ai/config" "" "200"
    
    # Test model list
    test_api "GET" "/api/v1/ai/models/list" "" "200"
    
    # Test face list
    test_api "GET" "/api/v1/ai/faces/list" "" "200"
}

# Test AI detection control
test_ai_detection() {
    print_info "Testing AI detection control..."
    
    # Test start detection
    local start_data='{
        "stream_id": "test_camera",
        "confidence_threshold": 0.6,
        "object_detection": true,
        "face_recognition": true,
        "plate_recognition": true
    }'
    
    test_api "POST" "/api/v1/ai/detection/start" "$start_data" "200"
    
    # Test stop detection
    local stop_data='{
        "stream_id": "test_camera"
    }'
    
    test_api "POST" "/api/v1/ai/detection/stop" "$stop_data" "200"
}

# Test stream management
test_stream_management() {
    print_info "Testing stream management..."
    
    # Test get streams
    test_api "GET" "/api/v1/getStreams" "" "200"
    
    # Test get stream info (this might return 404 if no streams exist)
    test_api "GET" "/api/v1/getStreamInfo?path=/live/test" "" "404" || true
}

# Test recording API
test_recording() {
    print_info "Testing recording functionality..."
    
    # Test get record list
    test_api "GET" "/api/v1/record/list" "" "200"
}

# Main test function
run_tests() {
    echo "=== SmartNVR Test Suite ==="
    echo "Testing SmartNVR functionality and AI API endpoints"
    echo ""
    
    # Check server
    check_server
    
    # Test basic functionality
    test_server_info
    
    # Test AI endpoints
    test_ai_endpoints
    
    # Test AI detection
    test_ai_detection
    
    # Test stream management
    test_stream_management
    
    # Test recording
    test_recording
    
    echo "=== Test Summary ==="
    print_success "All basic tests completed!"
    echo ""
    echo "Next steps:"
    echo "1. Install OpenCV: ./setup-smartnvr.sh"
    echo "2. Add real camera streams"
    echo "3. Test AI detection with actual video"
    echo "4. Configure face database"
    echo ""
}

# Show usage
show_usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -s, --server   Server host (default: localhost)"
    echo "  -p, --port     Server port (default: 80)"
    echo ""
    echo "Examples:"
    echo "  $0                    # Test with default settings"
    echo "  $0 -s 192.168.1.100  # Test remote server"
    echo "  $0 -p 8080           # Test different port"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -s|--server)
            SERVER_HOST="$2"
            BASE_URL="http://${SERVER_HOST}:${SERVER_PORT}"
            shift 2
            ;;
        -p|--port)
            SERVER_PORT="$2"
            BASE_URL="http://${SERVER_HOST}:${SERVER_PORT}"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Run the tests
run_tests
