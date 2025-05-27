#ifndef YOLO_DETECTOR_H
#define YOLO_DETECTOR_H

#ifdef ENABLE_AI

#include "AIDetector.h"
#include <opencv2/dnn.hpp>

namespace AI {

class YOLODetector : public AIDetector {
public:
    YOLODetector();
    virtual ~YOLODetector();
    
    // Initialize YOLO detector
    bool init(const AIConfig& config) override;
    
    // Detect objects in frame
    std::vector<DetectionResult> detect(const cv::Mat& frame, const std::string& stream_id) override;
    
    // Update configuration
    bool updateConfig(const AIConfig& config) override;

private:
    cv::dnn::Net net_;
    std::vector<std::string> class_names_;
    int input_width_ = 640;
    int input_height_ = 640;
    bool model_loaded_ = false;
    
    // YOLO-specific parameters
    float conf_threshold_ = 0.5f;
    float nms_threshold_ = 0.4f;
    
    // Load YOLO model
    bool loadModel(const std::string& model_path);
    
    // Load class names
    bool loadClassNames(const std::string& names_file);
    
    // Preprocess input image
    cv::Mat preprocess(const cv::Mat& image);
    
    // Postprocess YOLO output
    std::vector<DetectionResult> postprocess(const std::vector<cv::Mat>& outputs, 
                                           const cv::Mat& original_image,
                                           const std::string& stream_id);
    
    // Get output layer names
    std::vector<std::string> getOutputLayerNames();
    
    // Default COCO class names
    void initDefaultClassNames();
};

} // namespace AI

#endif // ENABLE_AI

#endif // YOLO_DETECTOR_H
