#ifdef ENABLE_AI

#include "AIManager.h"
#include "Log/Logger.h"
#include "Util/TimeClock.h"
#include "Common/Config.h"
#include <fstream>
#include <chrono>

namespace AI {

AIManager* AIManager::instance_ = nullptr;
std::mutex AIManager::instance_mutex_;

AIManager* AIManager::instance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = new AIManager();
    }
    return instance_;
}

AIManager::AIManager() : running_(false) {
    // Initialize detectors
    yolo_detector_ = std::make_shared<YOLODetector>();
    face_detector_ = std::make_shared<FaceDetector>();
    plate_detector_ = std::make_shared<PlateDetector>();
}

AIManager::~AIManager() {
    stop();
}

bool AIManager::init() {
    logInfo << "Initializing AI Manager...";
    
    // Load configuration
    if (!loadConfig()) {
        logError << "Failed to load AI configuration";
        return false;
    }
    
    // Initialize detectors with default config
    AIConfig default_config;
    
    try {
        if (!yolo_detector_->init(default_config)) {
            logWarn << "Failed to initialize YOLO detector, object detection will be disabled";
        }
        
        if (!face_detector_->init(default_config)) {
            logWarn << "Failed to initialize face detector, face recognition will be disabled";
        }
        
        if (!plate_detector_->init(default_config)) {
            logWarn << "Failed to initialize plate detector, plate recognition will be disabled";
        }
        
        logInfo << "AI Manager initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        logError << "Exception during AI Manager initialization: " << e.what();
        return false;
    }
}

void AIManager::start() {
    if (running_) {
        logWarn << "AI Manager is already running";
        return;
    }
    
    running_ = true;
    
    // Start worker threads
    for (int i = 0; i < num_threads_; ++i) {
        worker_threads_.emplace_back(&AIManager::workerThread, this);
    }
    
    logInfo << "AI Manager started with " << num_threads_ << " worker threads";
}

void AIManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Notify all worker threads
    queue_cv_.notify_all();
    
    // Wait for all threads to finish
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
    
    // Clear task queue
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!task_queue_.empty()) {
        task_queue_.pop();
    }
    
    logInfo << "AI Manager stopped";
}

bool AIManager::enableStreamAI(const std::string& stream_id, const AIConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    StreamAIConfig stream_config(stream_id);
    stream_config.ai_config = config;
    stream_config.enabled = true;
    
    stream_configs_[stream_id] = stream_config;
    
    logInfo << "AI enabled for stream: " << stream_id;
    return true;
}

bool AIManager::disableStreamAI(const std::string& stream_id) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto it = stream_configs_.find(stream_id);
    if (it != stream_configs_.end()) {
        it->second.enabled = false;
        logInfo << "AI disabled for stream: " << stream_id;
        return true;
    }
    
    logWarn << "Stream not found: " << stream_id;
    return false;
}

void AIManager::processFrame(const FrameBuffer::Ptr& frame, const std::string& stream_id) {
    if (!running_ || !frame) {
        return;
    }
    
    // Check if AI is enabled for this stream
    if (!shouldProcessStream(stream_id)) {
        return;
    }
    
    // Convert frame to OpenCV Mat
    cv::Mat mat = frameBufferToMat(frame);
    if (mat.empty()) {
        return;
    }
    
    // Add task to queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.emplace(mat, stream_id, TimeClock::now());
    }
    
    queue_cv_.notify_one();
}

void AIManager::workerThread() {
    logInfo << "AI worker thread started";
    
    while (running_) {
        AITask task("", "", 0);
        
        // Get task from queue
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !task_queue_.empty() || !running_; });
            
            if (!running_) {
                break;
            }
            
            if (!task_queue_.empty()) {
                task = task_queue_.front();
                task_queue_.pop();
            } else {
                continue;
            }
        }
        
        // Process the task
        processTask(task);
    }
    
    logInfo << "AI worker thread stopped";
}

void AIManager::processTask(const AITask& task) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    int total_detections = 0;
    int total_faces = 0;
    int total_plates = 0;
    
    try {
        // Get stream configuration
        StreamAIConfig stream_config;
        {
            std::lock_guard<std::mutex> lock(config_mutex_);
            auto it = stream_configs_.find(task.stream_id);
            if (it != stream_configs_.end()) {
                stream_config = it->second;
            }
        }
        
        if (!stream_config.enabled) {
            return;
        }
        
        // Object detection
        if (stream_config.ai_config.object_detection_enabled && yolo_detector_) {
            auto detections = yolo_detector_->detect(task.frame, task.stream_id);
            total_detections += detections.size();
        }
        
        // Face recognition
        if (stream_config.ai_config.face_recognition_enabled && face_detector_) {
            auto faces = face_detector_->detectFaces(task.frame, task.stream_id);
            total_faces += faces.size();
        }
        
        // Plate recognition
        if (stream_config.ai_config.plate_recognition_enabled && plate_detector_) {
            auto plates = plate_detector_->detectPlates(task.frame, task.stream_id);
            total_plates += plates.size();
        }
        
    } catch (const std::exception& e) {
        logError << "Exception during AI processing: " << e.what();
    }
    
    // Update statistics
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    updateStats(duration.count(), total_detections, total_faces, total_plates);
}

cv::Mat AIManager::frameBufferToMat(const FrameBuffer::Ptr& frame) {
    // This is a simplified implementation
    // In a real implementation, you'd need to handle different pixel formats
    // and properly convert from the frame buffer format to OpenCV Mat
    
    if (!frame || frame->size() == 0) {
        return cv::Mat();
    }
    
    // For now, assume a simple RGB format
    // You'll need to implement proper format conversion based on your frame format
    int width = 640;  // These should come from frame metadata
    int height = 480;
    
    cv::Mat mat(height, width, CV_8UC3);
    if (frame->size() >= mat.total() * mat.elemSize()) {
        memcpy(mat.data, frame->data(), mat.total() * mat.elemSize());
    }
    
    return mat;
}

bool AIManager::shouldProcessStream(const std::string& stream_id) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto it = stream_configs_.find(stream_id);
    if (it == stream_configs_.end() || !it->second.enabled) {
        return false;
    }
    
    int64_t now = TimeClock::now();
    int64_t interval = it->second.ai_config.detection_interval_ms;
    
    if (now - it->second.last_detection_time >= interval) {
        it->second.last_detection_time = now;
        return true;
    }
    
    return false;
}

void AIManager::updateStats(double processing_time_ms, int detections, int faces, int plates) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_.total_frames_processed++;
    stats_.total_detections += detections;
    stats_.total_faces_detected += faces;
    stats_.total_plates_detected += plates;
    
    // Update average processing time
    stats_.avg_processing_time_ms = 
        (stats_.avg_processing_time_ms * (stats_.total_frames_processed - 1) + processing_time_ms) / 
        stats_.total_frames_processed;
    
    stats_.active_streams = stream_configs_.size();
}

bool AIManager::loadConfig(const std::string& config_file) {
    try {
        // For now, use default configuration
        // In a real implementation, you'd load from the JSON file
        logInfo << "Loading AI configuration from: " << config_file;
        
        // This would be implemented to read from smartnvr.json
        // For now, just return true to indicate success
        return true;
        
    } catch (const std::exception& e) {
        logError << "Failed to load AI configuration: " << e.what();
        return false;
    }
}

AIManager::AIStats AIManager::getStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void AIManager::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = AIStats();
}

// Placeholder implementations for other methods
StreamAIConfig AIManager::getStreamConfig(const std::string& stream_id) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = stream_configs_.find(stream_id);
    if (it != stream_configs_.end()) {
        return it->second;
    }
    return StreamAIConfig();
}

bool AIManager::updateStreamConfig(const std::string& stream_id, const AIConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = stream_configs_.find(stream_id);
    if (it != stream_configs_.end()) {
        it->second.ai_config = config;
        return true;
    }
    return false;
}

std::vector<std::string> AIManager::getActiveStreams() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    std::vector<std::string> streams;
    for (const auto& pair : stream_configs_) {
        if (pair.second.enabled) {
            streams.push_back(pair.first);
        }
    }
    return streams;
}

void AIManager::setDetectionCallback(AIDetector::DetectionCallback callback) {
    detection_callback_ = callback;
    if (yolo_detector_) {
        yolo_detector_->setDetectionCallback(callback);
    }
}

void AIManager::setFaceCallback(AIDetector::FaceCallback callback) {
    face_callback_ = callback;
    if (face_detector_) {
        face_detector_->setFaceCallback(callback);
    }
}

void AIManager::setPlateCallback(AIDetector::PlateCallback callback) {
    plate_callback_ = callback;
    if (plate_detector_) {
        plate_detector_->setPlateCallback(callback);
    }
}

bool AIManager::addPerson(const std::string& person_id, const std::string& person_name, 
                         const std::vector<cv::Mat>& face_images) {
    if (face_detector_) {
        return face_detector_->addPerson(person_id, person_name, face_images);
    }
    return false;
}

bool AIManager::removePerson(const std::string& person_id) {
    if (face_detector_) {
        return face_detector_->removePerson(person_id);
    }
    return false;
}

std::vector<std::pair<std::string, std::string>> AIManager::getRegisteredPersons() const {
    if (face_detector_) {
        return face_detector_->getRegisteredPersons();
    }
    return {};
}

} // namespace AI

#endif // ENABLE_AI
