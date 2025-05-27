#ifndef AI_MANAGER_H
#define AI_MANAGER_H

#ifdef ENABLE_AI

#include "AIDetector.h"
#include "YOLODetector.h"
#include "FaceDetector.h"
#include "PlateDetector.h"
#include "Common/MediaSource.h"
#include "Common/Frame.h"
#include "EventLoopPool.h"
#include "WorkPoller/WorkLoopPool.h"
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace AI {

// AI processing task
struct AITask {
    cv::Mat frame;
    std::string stream_id;
    int64_t timestamp;

    AITask(const cv::Mat& f, const std::string& id, int64_t ts)
        : frame(f), stream_id(id), timestamp(ts) {}
};

// Stream AI configuration
struct StreamAIConfig {
    std::string stream_id;
    bool enabled = true;
    AIConfig ai_config;
    int64_t last_detection_time = 0;

    StreamAIConfig() = default;
    StreamAIConfig(const std::string& id) : stream_id(id) {}
};

class AIManager {
public:
    static AIManager* instance();

    ~AIManager();

    // Initialize AI manager
    bool init();

    // Start/stop AI processing
    void start();
    void stop();

    // Enable/disable AI for a specific stream
    bool enableStreamAI(const std::string& stream_id, const AIConfig& config);
    bool disableStreamAI(const std::string& stream_id);

    // Process a frame from a stream
    void processFrame(const FrameBuffer::Ptr& frame, const std::string& stream_id);

    // Get AI configuration for a stream
    StreamAIConfig getStreamConfig(const std::string& stream_id);

    // Update AI configuration for a stream
    bool updateStreamConfig(const std::string& stream_id, const AIConfig& config);

    // Get all active streams with AI enabled
    std::vector<std::string> getActiveStreams() const;

    // Set detection callbacks
    void setDetectionCallback(AIDetector::DetectionCallback callback);
    void setFaceCallback(AIDetector::FaceCallback callback);
    void setPlateCallback(AIDetector::PlateCallback callback);

    // Face database management
    bool addPerson(const std::string& person_id, const std::string& person_name,
                   const std::vector<cv::Mat>& face_images);
    bool removePerson(const std::string& person_id);
    std::vector<std::pair<std::string, std::string>> getRegisteredPersons() const;

    // Statistics
    struct AIStats {
        uint64_t total_frames_processed = 0;
        uint64_t total_detections = 0;
        uint64_t total_faces_detected = 0;
        uint64_t total_plates_detected = 0;
        double avg_processing_time_ms = 0.0;
        int active_streams = 0;
    };

    AIStats getStats() const;
    void resetStats();

private:
    AIManager();

    static AIManager* instance_;
    static std::mutex instance_mutex_;

    // AI detectors
    std::shared_ptr<YOLODetector> yolo_detector_;
    std::shared_ptr<FaceDetector> face_detector_;
    std::shared_ptr<PlateDetector> plate_detector_;

    // Stream configurations
    std::unordered_map<std::string, StreamAIConfig> stream_configs_;
    mutable std::mutex config_mutex_;

    // Processing thread pool
    std::vector<std::thread> worker_threads_;
    std::queue<AITask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> running_;
    int num_threads_ = 2;

    // Statistics
    mutable std::mutex stats_mutex_;
    AIStats stats_;

    // Callbacks
    AIDetector::DetectionCallback detection_callback_;
    AIDetector::FaceCallback face_callback_;
    AIDetector::PlateCallback plate_callback_;

    // Worker thread function
    void workerThread();

    // Process a single AI task
    void processTask(const AITask& task);

    // Convert frame buffer to OpenCV Mat
    cv::Mat frameBufferToMat(const FrameBuffer::Ptr& frame);

    // Check if stream should be processed (based on interval)
    bool shouldProcessStream(const std::string& stream_id);

    // Update processing statistics
    void updateStats(double processing_time_ms, int detections, int faces, int plates);

    // Load configuration from file
    bool loadConfig(const std::string& config_file = "conf/smartnvr.json");
};

} // namespace AI

#endif // ENABLE_AI

#endif // AI_MANAGER_H
