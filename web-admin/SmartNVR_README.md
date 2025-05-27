# SmartNVR Web Interface

A modern, feature-rich Network Video Recorder (NVR) web interface built with Vue.js 3 and Element Plus, designed for intelligent video surveillance systems with AI capabilities.

## Features

### 🎯 Core Features
- **Real-time Video Monitoring**: Multi-view live streaming with 1x1, 2x2, 3x3, and 4x4 grid layouts
- **AI-Powered Detection**: YOLOv8+ object detection, face recognition, and license plate recognition
- **Event Management**: Comprehensive event recording, filtering, and playback
- **Face Database**: Advanced face recognition database with person management
- **Camera Management**: Complete camera configuration and control system
- **PTZ Control**: Pan-tilt-zoom camera control with preset positions

### 🤖 AI Capabilities
- **Object Detection**: Real-time detection of people, vehicles, and objects
- **Face Recognition**: Identify known individuals from face database
- **License Plate Recognition**: Automatic vehicle plate detection and OCR
- **Motion Detection**: Advanced motion analysis and alerts
- **Intrusion Detection**: Perimeter security and zone monitoring

### 📊 Dashboard & Analytics
- **Real-time Statistics**: Live system metrics and performance monitoring
- **Event Timeline**: Chronological view of all security events
- **System Health**: Monitor AI services, recording, and streaming status
- **Resource Usage**: CPU, memory, and storage utilization tracking

### 🎨 Modern UI/UX
- **Responsive Design**: Works seamlessly on desktop, tablet, and mobile
- **Dark/Light Themes**: Multiple color schemes for different environments
- **Intuitive Navigation**: Clean, organized interface with breadcrumb navigation
- **Real-time Updates**: Live data refresh and WebSocket integration

## Technology Stack

### Frontend
- **Vue.js 3**: Progressive JavaScript framework with Composition API
- **Element Plus**: Enterprise-class UI components
- **Vue Router**: Client-side routing
- **Pinia**: State management
- **ECharts**: Data visualization and charts
- **Axios**: HTTP client for API communication

### Backend Integration
- **RTSP/ONVIF Support**: Industry-standard camera protocols
- **GB28181**: Chinese national standard for video surveillance
- **WebRTC**: Low-latency streaming
- **RESTful APIs**: Clean API design for all operations

## File Structure

```
web-admin/src/views/smartnvr/
├── Dashboard.vue          # Main SmartNVR dashboard
├── LiveView.vue          # Real-time video monitoring
├── AIManagement.vue      # AI model and detection management
├── EventsRecordings.vue  # Event timeline and recordings
├── FaceDatabase.vue      # Face recognition database
└── CameraManagement.vue  # Camera configuration and control
```

## Key Components

### 1. SmartNVR Dashboard (`Dashboard.vue`)
- **System Overview**: Real-time statistics and health monitoring
- **Live Preview**: Quick view of camera feeds
- **Recent Events**: Latest AI detections and alerts
- **Quick Actions**: Fast access to all major functions

### 2. Live View (`LiveView.vue`)
- **Multi-view Layouts**: 1x1, 2x2, 3x3, 4x4 grid options
- **AI Overlay**: Real-time detection boxes and labels
- **PTZ Controls**: Camera movement and zoom controls
- **Recording Controls**: Start/stop recording, snapshots
- **Audio Management**: Enable/disable audio streams

### 3. AI Management (`AIManagement.vue`)
- **Model Management**: Load/unload AI models
- **Detection Configuration**: Confidence thresholds and intervals
- **Stream Assignment**: Enable AI on specific cameras
- **Performance Monitoring**: AI processing statistics

### 4. Events & Recordings (`EventsRecordings.vue`)
- **Event Timeline**: Chronological event display
- **Advanced Filtering**: Filter by type, camera, date, confidence
- **Multiple Views**: Timeline, grid, and list layouts
- **Video Playback**: Built-in video player with controls
- **Export Functions**: Download events and generate reports

### 5. Face Database (`FaceDatabase.vue`)
- **Person Management**: Add, edit, delete personnel records
- **Face Gallery**: Multiple photos per person
- **Recognition History**: Track when and where people were detected
- **Batch Operations**: Import/export face data
- **Search & Filter**: Quick person lookup

### 6. Camera Management (`CameraManagement.vue`)
- **Camera Discovery**: Network scanning for IP cameras
- **Protocol Support**: RTSP, ONVIF, GB28181, HTTP
- **Configuration**: Resolution, encoding, authentication
- **Feature Control**: AI, recording, PTZ, audio settings
- **Health Monitoring**: Connection status and diagnostics

## Installation & Setup

### Prerequisites
- Node.js 16+ and npm/yarn
- Modern web browser with WebRTC support
- Backend media server (ZLMediaKit or similar)

### Development Setup
```bash
# Navigate to web-admin directory
cd web-admin

# Install dependencies
npm install

# Start development server
npm run dev

# Build for production
npm run build
```

### Configuration
1. Update API endpoints in `src/api/index.js`
2. Configure camera protocols in router settings
3. Set up WebSocket connections for real-time updates
4. Configure AI model paths and parameters

## API Integration

The interface expects the following API endpoints:

### Camera Management
- `GET /api/v1/cameras` - List all cameras
- `POST /api/v1/cameras` - Add new camera
- `PUT /api/v1/cameras/:id` - Update camera
- `DELETE /api/v1/cameras/:id` - Remove camera

### AI Management
- `GET /api/v1/ai/models` - List AI models
- `POST /api/v1/ai/models/load` - Load AI model
- `GET /api/v1/ai/detections` - Get detection results
- `PUT /api/v1/ai/config` - Update AI configuration

### Events & Recordings
- `GET /api/v1/events` - List events with filtering
- `GET /api/v1/events/:id/video` - Get event video
- `POST /api/v1/events/export` - Export events
- `DELETE /api/v1/events/:id` - Delete event

### Face Database
- `GET /api/v1/faces/persons` - List all persons
- `POST /api/v1/faces/persons` - Add new person
- `POST /api/v1/faces/upload` - Upload face images
- `GET /api/v1/faces/recognition` - Recognition history

## Browser Compatibility

- **Chrome/Edge**: Full support including WebRTC
- **Firefox**: Full support with WebRTC enabled
- **Safari**: Supported with some WebRTC limitations
- **Mobile Browsers**: Responsive design works on iOS/Android

## Security Features

- **Authentication**: User login and session management
- **Authorization**: Role-based access control
- **HTTPS**: Secure communication
- **Input Validation**: XSS and injection protection
- **Audit Logging**: Track all user actions

## Performance Optimization

- **Lazy Loading**: Components loaded on demand
- **Image Optimization**: Compressed thumbnails and previews
- **Caching**: Smart caching of static resources
- **WebSocket**: Efficient real-time communication
- **Virtual Scrolling**: Handle large datasets efficiently

## Customization

The interface is highly customizable:

- **Themes**: Modify CSS variables for custom colors
- **Layouts**: Adjust grid layouts and component sizes
- **Features**: Enable/disable specific functionality
- **Branding**: Update logos, titles, and styling
- **Languages**: Add internationalization support

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For technical support or feature requests:
- Create an issue on GitHub
- Check the documentation wiki
- Join the community discussions

---

**SmartNVR** - Intelligent Video Surveillance Made Simple
