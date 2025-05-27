#!/bin/bash

# SmartNVR Setup Script
# This script installs dependencies and sets up the SmartNVR environment

set -e

echo "=== SmartNVR Setup Script ==="
echo "Setting up AI-powered Network Video Recorder..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   print_error "This script should not be run as root"
   exit 1
fi

# Detect OS
if [[ -f /etc/os-release ]]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    print_error "Cannot detect OS"
    exit 1
fi

print_status "Detected OS: $OS $VER"

# Install system dependencies
install_dependencies() {
    print_status "Installing system dependencies..."
    
    if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
        sudo apt update
        sudo apt install -y \
            build-essential \
            cmake \
            git \
            pkg-config \
            libopencv-dev \
            libopencv-contrib-dev \
            python3-opencv \
            wget \
            unzip \
            curl
            
    elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]] || [[ "$OS" == *"Rocky"* ]]; then
        sudo yum update -y
        sudo yum groupinstall -y "Development Tools"
        sudo yum install -y \
            cmake \
            git \
            pkgconfig \
            opencv-devel \
            wget \
            unzip \
            curl
            
    else
        print_warning "Unsupported OS. Please install OpenCV manually."
        return 1
    fi
    
    print_status "System dependencies installed successfully"
}

# Download AI models
download_models() {
    print_status "Setting up AI models directory..."
    
    mkdir -p models
    cd models
    
    print_status "Downloading YOLO models..."
    # Download YOLOv8 models (these are example URLs, replace with actual model URLs)
    if [[ ! -f "yolov8n.onnx" ]]; then
        print_status "Downloading YOLOv8 nano model..."
        # wget -O yolov8n.onnx "https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.onnx"
        # For now, create placeholder files
        touch yolov8n.onnx
        print_warning "Placeholder model created. Please download actual YOLOv8 ONNX model."
    fi
    
    print_status "Setting up face recognition models..."
    if [[ ! -f "retinaface.onnx" ]]; then
        touch retinaface.onnx
        print_warning "Placeholder face detection model created."
    fi
    
    if [[ ! -f "arcface.onnx" ]]; then
        touch arcface.onnx
        print_warning "Placeholder face recognition model created."
    fi
    
    print_status "Setting up license plate models..."
    if [[ ! -f "plate_detect.onnx" ]]; then
        touch plate_detect.onnx
        print_warning "Placeholder plate detection model created."
    fi
    
    if [[ ! -f "plate_ocr.onnx" ]]; then
        touch plate_ocr.onnx
        print_warning "Placeholder plate OCR model created."
    fi
    
    cd ..
    print_status "AI models directory setup complete"
}

# Setup ONNX Runtime
setup_onnxruntime() {
    print_status "Setting up ONNX Runtime..."
    
    mkdir -p Thirdparty/onnxruntime
    cd Thirdparty/onnxruntime
    
    # Download ONNX Runtime (CPU version)
    ONNX_VERSION="1.16.3"
    ONNX_PLATFORM="linux-x64"
    ONNX_PACKAGE="onnxruntime-${ONNX_PLATFORM}-${ONNX_VERSION}"
    
    if [[ ! -d "include" ]]; then
        print_status "Downloading ONNX Runtime ${ONNX_VERSION}..."
        wget -O "${ONNX_PACKAGE}.tgz" \
            "https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/${ONNX_PACKAGE}.tgz"
        
        tar -xzf "${ONNX_PACKAGE}.tgz"
        
        # Copy headers and libraries
        cp -r "${ONNX_PACKAGE}/include" .
        cp -r "${ONNX_PACKAGE}/lib" .
        
        # Cleanup
        rm -rf "${ONNX_PACKAGE}" "${ONNX_PACKAGE}.tgz"
        
        print_status "ONNX Runtime installed successfully"
    else
        print_status "ONNX Runtime already installed"
    fi
    
    cd ../..
}

# Enable AI in CMake
enable_ai() {
    print_status "Enabling AI features in CMake..."
    
    # Update CMakeLists.txt to enable AI
    sed -i 's/option(ENABLE_AI "Enable AI detection (OpenCV, ONNX)" false)/option(ENABLE_AI "Enable AI detection (OpenCV, ONNX)" true)/' CMakeLists.txt
    
    print_status "AI features enabled"
}

# Build project with AI
build_project() {
    print_status "Building SmartNVR with AI support..."
    
    mkdir -p build
    cd build
    
    cmake .. -DENABLE_AI=ON
    make -j$(nproc)
    
    if [[ $? -eq 0 ]]; then
        print_status "SmartNVR built successfully!"
    else
        print_error "Build failed. Please check the error messages above."
        exit 1
    fi
    
    cd ..
}

# Create directories
setup_directories() {
    print_status "Setting up SmartNVR directories..."
    
    mkdir -p recordings
    mkdir -p logs
    mkdir -p database
    
    # Copy configuration files
    cp conf/smartnvr.json build/
    
    print_status "Directory structure created"
}

# Main installation process
main() {
    print_status "Starting SmartNVR setup..."
    
    # Check if OpenCV is already installed
    if pkg-config --exists opencv4 || pkg-config --exists opencv; then
        print_status "OpenCV is already installed"
    else
        print_status "OpenCV not found, installing..."
        install_dependencies
    fi
    
    # Setup ONNX Runtime
    setup_onnxruntime
    
    # Download AI models
    download_models
    
    # Enable AI features
    enable_ai
    
    # Setup directories
    setup_directories
    
    # Build project
    build_project
    
    print_status "SmartNVR setup completed successfully!"
    echo ""
    echo "=== Next Steps ==="
    echo "1. Download actual AI models and place them in the 'models' directory"
    echo "2. Configure your cameras in conf/smartnvr.json"
    echo "3. Start the SmartNVR server: cd build && ./SimpleMediaServer"
    echo "4. Access the web interface at http://localhost:8080"
    echo ""
    echo "For more information, see the smartnvr_plan.md file."
}

# Run main function
main "$@"
