#ifndef PLATE_DETECTOR_H
#define PLATE_DETECTOR_H

#ifdef ENABLE_AI

#include "AIDetector.h"
#include <opencv2/dnn.hpp>

namespace AI {

class PlateDetector : public AIDetector {
public:
    PlateDetector();
    virtual ~PlateDetector();
    
    // Initialize plate detector
    bool init(const AIConfig& config) override;
    
    // Detect license plates in frame (returns as DetectionResult for compatibility)
    std::vector<DetectionResult> detect(const cv::Mat& frame, const std::string& stream_id) override;
    
    // Plate-specific detection method
    std::vector<PlateResult> detectPlates(const cv::Mat& frame, const std::string& stream_id);
    
    // Set plate recognition callback
    void setPlateCallback(PlateCallback callback) { plate_callback_ = callback; }
    
    // Update configuration
    bool updateConfig(const AIConfig& config) override;

private:
    cv::dnn::Net detection_net_;
    cv::dnn::Net recognition_net_;
    bool detection_model_loaded_ = false;
    bool recognition_model_loaded_ = false;
    
    PlateCallback plate_callback_;
    
    // Detection parameters
    float detection_threshold_ = 0.5f;
    int input_width_ = 640;
    int input_height_ = 640;
    
    // Recognition parameters
    int plate_width_ = 168;
    int plate_height_ = 48;
    std::vector<std::string> char_dict_;
    
    // Load plate detection model
    bool loadDetectionModel(const std::string& model_path);
    
    // Load plate recognition model (OCR)
    bool loadRecognitionModel(const std::string& model_path);
    
    // Detect license plate boxes
    std::vector<cv::Rect> detectPlateBoxes(const cv::Mat& image);
    
    // Recognize text from plate image
    std::string recognizePlateText(const cv::Mat& plate_image);
    
    // Initialize character dictionary for OCR
    void initCharDictionary();
    
    // Preprocess plate image for OCR
    cv::Mat preprocessPlateForOCR(const cv::Mat& plate);
    
    // Postprocess OCR output to get text
    std::string postprocessOCR(const cv::Mat& output);
    
    // Validate plate text format (basic validation)
    bool isValidPlateText(const std::string& text);
    
    // Clean up plate text (remove invalid characters)
    std::string cleanPlateText(const std::string& text);
};

} // namespace AI

#endif // ENABLE_AI

#endif // PLATE_DETECTOR_H
