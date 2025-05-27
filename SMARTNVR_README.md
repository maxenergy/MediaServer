# SmartNVR - AI-Powered Network Video Recorder

## Overview

SmartNVR is an advanced Network Video Recorder built on top of the SimpleMediaServer platform, featuring AI-powered object detection, face recognition, and license plate recognition capabilities. It supports multiple camera protocols and provides intelligent video analytics for security and surveillance applications.

## Features

### 🎯 Core Capabilities
- **Multi-Protocol Support**: RTSP, ONVIF, GB28181, RTMP, WebRTC, SRT
- **AI-Powered Detection**: YOLOv8+ object detection, face recognition, license plate recognition
- **Smart Recording**: Event-driven recording with pre/post-event buffering
- **Real-time Analytics**: Live video analysis with configurable detection intervals
- **Web Management**: Comprehensive web interface for monitoring and configuration

### 🤖 AI Detection Features
- **Object Detection**: Person, vehicle, animal detection using YOLO models
- **Face Recognition**: Face detection and identification with database management
- **License Plate Recognition**: Automatic plate detection and OCR
- **Motion Detection**: Advanced motion analysis with configurable sensitivity
- **Event Triggers**: Customizable alerts and actions based on detection results

### 📹 Recording & Storage
- **Event-Based Recording**: Automatic recording triggered by AI detections
- **Continuous Recording**: Traditional 24/7 recording mode
- **Multiple Formats**: MP4, PS format support
- **Intelligent Storage**: Automatic cleanup and retention policies
- **Metadata Tagging**: AI detection results embedded in recordings

### 🌐 Network & Protocols
- **RTSP Cameras**: Standard IP camera support
- **ONVIF Compliance**: Auto-discovery and PTZ control
- **GB28181**: Chinese national standard support
- **Multi-Camera**: Support for up to 64 concurrent cameras
- **Load Balancing**: Intelligent resource distribution

## Quick Start

### Prerequisites
- Linux system (Ubuntu 18.04+ recommended)
- GCC 8+ compiler
- CMake 3.0+
- OpenCV 4.0+ (will be installed by setup script)

### Installation

1. **Clone and setup the SmartNVR branch:**
   ```bash
   git checkout smartnvr
   ```

2. **Run the setup script:**
   ```bash
   ./setup-smartnvr.sh
   ```

3. **Start the SmartNVR server:**
   ```bash
   cd build
   ./SimpleMediaServer
   ```

4. **Access the web interface:**
   Open your browser and go to `http://localhost:8080`

### Manual Installation

If you prefer manual installation:

1. **Install dependencies:**
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake git pkg-config libopencv-dev
   ```

2. **Enable AI features:**
   ```bash
   # Edit CMakeLists.txt and set ENABLE_AI to true
   sed -i 's/ENABLE_AI.*false/ENABLE_AI "Enable AI detection" true/' CMakeLists.txt
   ```

3. **Build the project:**
   ```bash
   mkdir build && cd build
   cmake .. -DENABLE_AI=ON
   make -j4
   ```

## Configuration

### SmartNVR Configuration File

The main configuration is in `conf/smartnvr.json`:

```json
{
  "SmartNVR": {
    "AI": {
      "enabled": true,
      "confidence_threshold": 0.5,
      "detection_interval_ms": 1000,
      "models": {
        "object_detection": {
          "model_path": "models/yolov8n.onnx",
          "target_classes": ["person", "car", "truck"]
        },
        "face_recognition": {
          "detection_model": "models/retinaface.onnx",
          "recognition_model": "models/arcface.onnx"
        }
      }
    },
    "Recording": {
      "event_based": true,
      "pre_record_seconds": 10,
      "post_record_seconds": 30,
      "retention_days": 30
    }
  }
}
```

### Camera Configuration

Add cameras via the web interface or API:

```bash
# Add RTSP camera
curl -X POST http://localhost/api/v1/ai/stream/enable \
  -H "Content-Type: application/json" \
  -d '{
    "stream_id": "camera1",
    "rtsp_url": "rtsp://192.168.1.100:554/stream1",
    "object_detection": true,
    "face_recognition": true
  }'
```

## API Reference

### AI Detection Control

- **Start AI Detection:**
  ```
  POST /api/v1/ai/detection/start
  {
    "stream_id": "camera1",
    "confidence_threshold": 0.6,
    "object_detection": true,
    "face_recognition": true
  }
  ```

- **Get AI Statistics:**
  ```
  GET /api/v1/ai/stats
  ```

- **List Active Streams:**
  ```
  GET /api/v1/ai/detection/status
  ```

### Face Database Management

- **Add Person:**
  ```
  POST /api/v1/ai/faces/add
  {
    "person_id": "john_doe",
    "person_name": "John Doe",
    "face_images": ["base64_encoded_image"]
  }
  ```

- **List Persons:**
  ```
  GET /api/v1/ai/faces/list
  ```

## AI Models

### Supported Models

1. **Object Detection:**
   - YOLOv8 (nano, small, medium, large)
   - YOLOv9, YOLOv10
   - Custom trained models

2. **Face Recognition:**
   - RetinaFace (detection)
   - ArcFace (recognition)
   - Custom face models

3. **License Plate Recognition:**
   - YOLO-based plate detection
   - CRNN-based OCR

### Model Installation

Download pre-trained models:

```bash
# Create models directory
mkdir models

# Download YOLOv8 (example)
wget -O models/yolov8n.onnx "https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.onnx"

# Download face models (replace with actual URLs)
wget -O models/retinaface.onnx "URL_TO_RETINAFACE_MODEL"
wget -O models/arcface.onnx "URL_TO_ARCFACE_MODEL"
```

## Performance Optimization

### Hardware Requirements

- **Minimum:** 4GB RAM, 2 CPU cores
- **Recommended:** 8GB+ RAM, 4+ CPU cores
- **GPU Support:** NVIDIA GPU with CUDA for acceleration

### Optimization Tips

1. **Adjust detection interval** based on your needs
2. **Use appropriate model sizes** (nano for real-time, larger for accuracy)
3. **Enable GPU acceleration** if available
4. **Configure frame skipping** for high-resolution streams

## Troubleshooting

### Common Issues

1. **OpenCV not found:**
   ```bash
   sudo apt install libopencv-dev libopencv-contrib-dev
   ```

2. **AI models not loading:**
   - Check model file paths in configuration
   - Ensure models are in ONNX format
   - Verify file permissions

3. **High CPU usage:**
   - Increase detection interval
   - Use smaller AI models
   - Enable frame skipping

### Logs

Check logs for debugging:
```bash
tail -f build/log/SimpleMediaServer.*.log
```

## Development

### Adding Custom AI Models

1. Convert your model to ONNX format
2. Update the detector classes in `Src/AI/`
3. Modify configuration schema
4. Test with sample streams

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For support and questions:
- Check the documentation in `smartnvr_plan.md`
- Review the API documentation
- Submit issues on GitHub

## Roadmap

- [ ] GPU acceleration support
- [ ] Advanced analytics (crowd detection, behavior analysis)
- [ ] Cloud storage integration
- [ ] Mobile app for monitoring
- [ ] Advanced PTZ control
- [ ] Multi-site management
