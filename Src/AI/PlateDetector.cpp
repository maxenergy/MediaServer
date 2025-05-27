#ifdef ENABLE_AI

#include "PlateDetector.h"
#include "Log/Logger.h"
#include "Util/TimeClock.h"
#include <algorithm>
#include <regex>

namespace AI {

PlateDetector::PlateDetector() {
    initCharDictionary();
}

PlateDetector::~PlateDetector() {
}

bool PlateDetector::init(const AIConfig& config) {
    config_ = config;
    
    if (!config_.plate_recognition_enabled) {
        logInfo << "License plate recognition is disabled";
        return true;
    }
    
    detection_threshold_ = config_.confidence_threshold;
    
    // Load detection model
    if (!loadDetectionModel(config_.plate_detection_model)) {
        logError << "Failed to load plate detection model: " << config_.plate_detection_model;
        return false;
    }
    
    // Load recognition model
    if (!loadRecognitionModel(config_.plate_recognition_model)) {
        logError << "Failed to load plate recognition model: " << config_.plate_recognition_model;
        return false;
    }
    
    logInfo << "Plate detector initialized successfully";
    return true;
}

bool PlateDetector::loadDetectionModel(const std::string& model_path) {
    try {
        logInfo << "Loading plate detection model: " << model_path;
        
        // Placeholder - would load actual YOLO or similar model for plate detection
        detection_model_loaded_ = true;
        return true;
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception while loading plate detection model: " << e.what();
        return false;
    }
}

bool PlateDetector::loadRecognitionModel(const std::string& model_path) {
    try {
        logInfo << "Loading plate recognition model: " << model_path;
        
        // Placeholder - would load actual CRNN model for OCR
        recognition_model_loaded_ = true;
        return true;
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception while loading plate recognition model: " << e.what();
        return false;
    }
}

std::vector<DetectionResult> PlateDetector::detect(const cv::Mat& frame, const std::string& stream_id) {
    std::vector<DetectionResult> results;
    
    if (!detection_model_loaded_ || !enabled_ || frame.empty()) {
        return results;
    }
    
    try {
        // Detect license plates
        std::vector<cv::Rect> plate_boxes = detectPlateBoxes(frame);
        
        int64_t timestamp = TimeClock::now();
        
        // Convert to DetectionResult format
        for (const auto& box : plate_boxes) {
            DetectionResult result;
            result.class_id = 1; // License plate class
            result.class_name = "license_plate";
            result.confidence = 0.7f; // Placeholder confidence
            result.bbox = box;
            result.timestamp = timestamp;
            result.stream_id = stream_id;
            
            results.push_back(result);
        }
        
        // Call callback if set
        if (detection_callback_ && !results.empty()) {
            detection_callback_(results);
        }
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception during plate detection: " << e.what();
    }
    
    return results;
}

std::vector<PlateResult> PlateDetector::detectPlates(const cv::Mat& frame, const std::string& stream_id) {
    std::vector<PlateResult> results;
    
    if (!detection_model_loaded_ || !enabled_ || frame.empty()) {
        return results;
    }
    
    try {
        // Detect license plates
        std::vector<cv::Rect> plate_boxes = detectPlateBoxes(frame);
        
        int64_t timestamp = TimeClock::now();
        
        // Process each detected plate
        for (const auto& box : plate_boxes) {
            PlateResult result;
            result.bbox = box;
            result.confidence = 0.7f; // Placeholder
            result.timestamp = timestamp;
            result.stream_id = stream_id;
            
            // Extract plate region
            cv::Mat plate_roi = frame(box);
            if (!plate_roi.empty() && recognition_model_loaded_) {
                // Recognize text
                result.plate_text = recognizePlateText(plate_roi);
                result.plate_text = cleanPlateText(result.plate_text);
                
                // Only add if we got valid text
                if (isValidPlateText(result.plate_text)) {
                    results.push_back(result);
                }
            }
        }
        
        // Call plate callback if set
        if (plate_callback_ && !results.empty()) {
            plate_callback_(results);
        }
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception during plate recognition: " << e.what();
    }
    
    return results;
}

std::vector<cv::Rect> PlateDetector::detectPlateBoxes(const cv::Mat& image) {
    std::vector<cv::Rect> plates;
    
    // Placeholder implementation using simple contour detection
    // In a real implementation, you'd use a trained YOLO model for plate detection
    
    cv::Mat gray, thresh;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    
    // Apply morphological operations to enhance plate-like regions
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(17, 3));
    cv::morphologyEx(gray, gray, cv::MORPH_BLACKHAT, kernel);
    
    // Threshold
    cv::threshold(gray, thresh, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Filter contours based on aspect ratio and area
    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        
        // Check aspect ratio (license plates are typically wider than tall)
        float aspect_ratio = static_cast<float>(rect.width) / rect.height;
        if (aspect_ratio > 2.0f && aspect_ratio < 6.0f) {
            // Check area
            if (rect.area() > 1000 && rect.area() < 50000) {
                plates.push_back(rect);
            }
        }
    }
    
    return plates;
}

std::string PlateDetector::recognizePlateText(const cv::Mat& plate_image) {
    // Placeholder implementation using simple OCR
    // In a real implementation, you'd use a trained CRNN model
    
    if (plate_image.empty()) {
        return "";
    }
    
    // Preprocess the plate image
    cv::Mat processed = preprocessPlateForOCR(plate_image);
    
    // Placeholder: generate a random-looking plate number
    // In reality, this would be the output of your OCR model
    static int counter = 0;
    counter++;
    
    std::string plate_text = "ABC" + std::to_string(1000 + (counter % 9000));
    
    return plate_text;
}

void PlateDetector::initCharDictionary() {
    // Initialize character dictionary for OCR
    char_dict_.clear();
    
    // Add digits
    for (char c = '0'; c <= '9'; ++c) {
        char_dict_.push_back(std::string(1, c));
    }
    
    // Add uppercase letters
    for (char c = 'A'; c <= 'Z'; ++c) {
        char_dict_.push_back(std::string(1, c));
    }
    
    // Add special characters commonly found on license plates
    char_dict_.push_back("-");
    char_dict_.push_back(" ");
}

cv::Mat PlateDetector::preprocessPlateForOCR(const cv::Mat& plate) {
    cv::Mat processed;
    
    // Resize to standard OCR input size
    cv::resize(plate, processed, cv::Size(plate_width_, plate_height_));
    
    // Convert to grayscale
    if (processed.channels() == 3) {
        cv::cvtColor(processed, processed, cv::COLOR_BGR2GRAY);
    }
    
    // Normalize
    processed.convertTo(processed, CV_32F, 1.0/255.0);
    
    return processed;
}

std::string PlateDetector::postprocessOCR(const cv::Mat& output) {
    // Placeholder for OCR postprocessing
    // In a real implementation, you'd decode the CRNN output
    return "";
}

bool PlateDetector::isValidPlateText(const std::string& text) {
    if (text.empty() || text.length() < 3) {
        return false;
    }
    
    // Basic validation - check if it contains alphanumeric characters
    std::regex plate_regex("^[A-Z0-9\\-\\s]+$");
    return std::regex_match(text, plate_regex);
}

std::string PlateDetector::cleanPlateText(const std::string& text) {
    std::string cleaned = text;
    
    // Remove extra spaces
    cleaned = std::regex_replace(cleaned, std::regex("\\s+"), " ");
    
    // Trim leading/trailing spaces
    cleaned.erase(0, cleaned.find_first_not_of(" \t\n\r\f\v"));
    cleaned.erase(cleaned.find_last_not_of(" \t\n\r\f\v") + 1);
    
    // Convert to uppercase
    std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::toupper);
    
    return cleaned;
}

bool PlateDetector::updateConfig(const AIConfig& config) {
    config_ = config;
    detection_threshold_ = config_.confidence_threshold;
    
    // Reload models if paths changed
    if (config_.plate_detection_model != config.plate_detection_model) {
        return loadDetectionModel(config_.plate_detection_model);
    }
    
    if (config_.plate_recognition_model != config.plate_recognition_model) {
        return loadRecognitionModel(config_.plate_recognition_model);
    }
    
    return true;
}

} // namespace AI

#endif // ENABLE_AI
