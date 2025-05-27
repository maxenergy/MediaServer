#!/bin/bash

# MediaServer Production Startup Script
# This script starts the C++ backend and serves the built frontend

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Check if we're in the right directory
if [ ! -f "build/SimpleMediaServer" ]; then
    print_error "SimpleMediaServer executable not found in build/ directory"
    exit 1
fi

if [ ! -d "web-admin/dist" ]; then
    print_error "Frontend build not found. Please run: cd web-admin && npm run build"
    exit 1
fi

print_header "MediaServer Production Startup"

# Function to cleanup processes on exit
cleanup() {
    print_warning "Shutting down services..."
    if [ ! -z "$BACKEND_PID" ]; then
        kill $BACKEND_PID 2>/dev/null || true
        print_status "Backend stopped"
    fi
    if [ ! -z "$FRONTEND_PID" ]; then
        kill $FRONTEND_PID 2>/dev/null || true
        print_status "Frontend server stopped"
    fi
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Check if configuration file exists
if [ ! -f "build/server.json" ]; then
    print_warning "Configuration file not found in build directory, copying from conf/"
    cp conf/server.json build/server.json
    cp conf/sms.com.* build/ 2>/dev/null || print_warning "SSL certificates not found"
fi

# Start the C++ backend
print_status "Starting C++ MediaServer backend..."
cd build
./SimpleMediaServer &
BACKEND_PID=$!
cd ..

# Wait a moment for backend to start
sleep 2

# Check if backend is running
if ! kill -0 $BACKEND_PID 2>/dev/null; then
    print_error "Failed to start MediaServer backend"
    exit 1
fi

print_status "Backend started successfully (PID: $BACKEND_PID)"

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    print_error "Node.js is not installed. Please install Node.js to serve the frontend."
    kill $BACKEND_PID
    exit 1
fi

# Start a simple HTTP server for the frontend
print_status "Starting production frontend server..."
cd web-admin/dist

# Use Node.js built-in server or Python if available
if command -v npx &> /dev/null; then
    # Use serve package if available, otherwise use Node.js built-in
    if npx serve --version &> /dev/null; then
        npx serve -s . -l 3000 &
        FRONTEND_PID=$!
    else
        # Install serve package
        print_status "Installing serve package..."
        npm install -g serve
        npx serve -s . -l 3000 &
        FRONTEND_PID=$!
    fi
elif command -v python3 &> /dev/null; then
    # Fallback to Python HTTP server
    python3 -m http.server 3000 &
    FRONTEND_PID=$!
elif command -v python &> /dev/null; then
    # Fallback to Python 2 HTTP server
    python -m SimpleHTTPServer 3000 &
    FRONTEND_PID=$!
else
    print_error "No suitable HTTP server found. Please install Node.js or Python."
    kill $BACKEND_PID
    exit 1
fi

cd ../..

# Wait a moment for frontend to start
sleep 3

# Check if frontend is running
if ! kill -0 $FRONTEND_PID 2>/dev/null; then
    print_error "Failed to start frontend server"
    kill $BACKEND_PID
    exit 1
fi

print_status "Frontend server started successfully (PID: $FRONTEND_PID)"

print_header "Production Services Started Successfully"
echo -e "${GREEN}Backend API:${NC}     http://localhost:80/api/v1/"
echo -e "${GREEN}Frontend Web:${NC}    http://localhost:3000/"
echo -e "${GREEN}RTSP Server:${NC}     rtsp://localhost:554/"
echo -e "${GREEN}RTMP Server:${NC}     rtmp://localhost:1935/"
echo -e "${GREEN}WebRTC Server:${NC}   http://localhost:80/api/v1/rtc/"
echo -e "${GREEN}SRT Server:${NC}      srt://localhost:6666/"
echo ""
echo -e "${BLUE}Production Mode:${NC} Serving optimized frontend build"
echo -e "${YELLOW}Press Ctrl+C to stop all services${NC}"

# Wait for processes
wait $BACKEND_PID $FRONTEND_PID
