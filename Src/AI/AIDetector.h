#ifndef AI_DETECTOR_H
#define AI_DETECTOR_H

#ifdef ENABLE_AI

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <opencv2/opencv.hpp>
#include "Common/Frame.h"
#include "Log/Logger.h"

namespace AI {

// Detection result structure
struct DetectionResult {
    int class_id;
    std::string class_name;
    float confidence;
    cv::Rect bbox;
    int64_t timestamp;
    std::string stream_id;
    
    DetectionResult() : class_id(-1), confidence(0.0f), timestamp(0) {}
};

// Face recognition result
struct FaceResult {
    cv::Rect bbox;
    float confidence;
    std::vector<float> embedding;
    std::string person_id;
    std::string person_name;
    int64_t timestamp;
    std::string stream_id;
    
    FaceResult() : confidence(0.0f), timestamp(0) {}
};

// License plate result
struct PlateResult {
    cv::Rect bbox;
    float confidence;
    std::string plate_text;
    int64_t timestamp;
    std::string stream_id;
    
    PlateResult() : confidence(0.0f), timestamp(0) {}
};

// AI detection configuration
struct AIConfig {
    bool enabled = true;
    float confidence_threshold = 0.5f;
    int detection_interval_ms = 1000;
    bool gpu_enabled = false;
    std::string model_path;
    std::vector<std::string> target_classes;
    
    // Object detection specific
    bool object_detection_enabled = true;
    std::string object_model_path = "models/yolov8n.onnx";
    
    // Face recognition specific
    bool face_recognition_enabled = true;
    std::string face_detection_model = "models/retinaface.onnx";
    std::string face_recognition_model = "models/arcface.onnx";
    
    // License plate specific
    bool plate_recognition_enabled = true;
    std::string plate_detection_model = "models/plate_detect.onnx";
    std::string plate_recognition_model = "models/plate_ocr.onnx";
};

// Base AI detector interface
class AIDetector {
public:
    using DetectionCallback = std::function<void(const std::vector<DetectionResult>&)>;
    using FaceCallback = std::function<void(const std::vector<FaceResult>&)>;
    using PlateCallback = std::function<void(const std::vector<PlateResult>&)>;
    
    AIDetector() = default;
    virtual ~AIDetector() = default;
    
    // Initialize the detector
    virtual bool init(const AIConfig& config) = 0;
    
    // Process a frame and return detection results
    virtual std::vector<DetectionResult> detect(const cv::Mat& frame, const std::string& stream_id) = 0;
    
    // Set detection callback
    virtual void setDetectionCallback(DetectionCallback callback) { detection_callback_ = callback; }
    
    // Enable/disable detection
    virtual void setEnabled(bool enabled) { enabled_ = enabled; }
    virtual bool isEnabled() const { return enabled_; }
    
    // Get configuration
    virtual const AIConfig& getConfig() const { return config_; }
    
    // Update configuration
    virtual bool updateConfig(const AIConfig& config) {
        config_ = config;
        return true;
    }

protected:
    AIConfig config_;
    bool enabled_ = true;
    DetectionCallback detection_callback_;
    
    // Utility function to convert frame to cv::Mat
    cv::Mat frameToMat(const FrameBuffer::Ptr& frame);
    
    // Utility function to resize image while maintaining aspect ratio
    cv::Mat resizeImage(const cv::Mat& image, int target_width, int target_height);
    
    // Non-maximum suppression
    std::vector<int> nms(const std::vector<cv::Rect>& boxes, 
                        const std::vector<float>& scores, 
                        float nms_threshold);
};

// Factory for creating AI detectors
class AIDetectorFactory {
public:
    enum DetectorType {
        YOLO_DETECTOR,
        FACE_DETECTOR,
        PLATE_DETECTOR
    };
    
    static std::shared_ptr<AIDetector> create(DetectorType type);
};

} // namespace AI

#endif // ENABLE_AI

#endif // AI_DETECTOR_H
