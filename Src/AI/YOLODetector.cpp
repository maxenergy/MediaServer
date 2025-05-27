#ifdef ENABLE_AI

#include "YOLODetector.h"
#include "Log/Logger.h"
#include "Util/TimeClock.h"
#include <fstream>

namespace AI {

YOLODetector::YOLODetector() {
    initDefaultClassNames();
}

YOLODetector::~YOLODetector() {
}

bool YOLODetector::init(const AIConfig& config) {
    config_ = config;
    
    if (!config_.object_detection_enabled) {
        logInfo << "YOLO object detection is disabled";
        return true;
    }
    
    conf_threshold_ = config_.confidence_threshold;
    
    // Load the model
    if (!loadModel(config_.object_model_path)) {
        logError << "Failed to load YOLO model: " << config_.object_model_path;
        return false;
    }
    
    logInfo << "YOLO detector initialized successfully";
    return true;
}

bool YOLODetector::loadModel(const std::string& model_path) {
    try {
        // Try to load ONNX model first
        if (model_path.find(".onnx") != std::string::npos) {
            net_ = cv::dnn::readNetFromONNX(model_path);
        } else if (model_path.find(".weights") != std::string::npos) {
            // Darknet format
            std::string config_file = model_path;
            config_file.replace(config_file.find(".weights"), 8, ".cfg");
            net_ = cv::dnn::readNetFromDarknet(config_file, model_path);
        } else {
            logError << "Unsupported model format: " << model_path;
            return false;
        }
        
        if (net_.empty()) {
            logError << "Failed to load neural network from: " << model_path;
            return false;
        }
        
        // Set backend and target
        if (config_.gpu_enabled) {
            net_.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            net_.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
            logInfo << "Using GPU backend for YOLO";
        } else {
            net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            logInfo << "Using CPU backend for YOLO";
        }
        
        model_loaded_ = true;
        logInfo << "YOLO model loaded successfully: " << model_path;
        return true;
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception while loading YOLO model: " << e.what();
        return false;
    } catch (const std::exception& e) {
        logError << "Exception while loading YOLO model: " << e.what();
        return false;
    }
}

void YOLODetector::initDefaultClassNames() {
    // COCO dataset class names
    class_names_ = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
        "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
        "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
        "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
        "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",
        "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
        "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
        "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
        "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier",
        "toothbrush"
    };
}

cv::Mat YOLODetector::preprocess(const cv::Mat& image) {
    cv::Mat blob;
    cv::dnn::blobFromImage(image, blob, 1.0/255.0, cv::Size(input_width_, input_height_), 
                          cv::Scalar(0, 0, 0), true, false, CV_32F);
    return blob;
}

std::vector<DetectionResult> YOLODetector::detect(const cv::Mat& frame, const std::string& stream_id) {
    std::vector<DetectionResult> results;
    
    if (!model_loaded_ || !enabled_ || frame.empty()) {
        return results;
    }
    
    try {
        // Preprocess
        cv::Mat blob = preprocess(frame);
        net_.setInput(blob);
        
        // Forward pass
        std::vector<cv::Mat> outputs;
        net_.forward(outputs, getOutputLayerNames());
        
        // Postprocess
        results = postprocess(outputs, frame, stream_id);
        
        // Call callback if set
        if (detection_callback_ && !results.empty()) {
            detection_callback_(results);
        }
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception during YOLO detection: " << e.what();
    } catch (const std::exception& e) {
        logError << "Exception during YOLO detection: " << e.what();
    }
    
    return results;
}

std::vector<DetectionResult> YOLODetector::postprocess(const std::vector<cv::Mat>& outputs, 
                                                      const cv::Mat& original_image,
                                                      const std::string& stream_id) {
    std::vector<DetectionResult> results;
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> class_ids;
    
    float x_scale = static_cast<float>(original_image.cols) / input_width_;
    float y_scale = static_cast<float>(original_image.rows) / input_height_;
    
    // Parse YOLO output
    for (const auto& output : outputs) {
        const float* data = (float*)output.data;
        int rows = output.size[1];
        int dimensions = output.size[2];
        
        for (int i = 0; i < rows; ++i) {
            float confidence = data[4];
            if (confidence >= conf_threshold_) {
                float* classes_scores = data + 5;
                cv::Mat scores(1, class_names_.size(), CV_32FC1, classes_scores);
                cv::Point class_id_point;
                double max_class_score;
                cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id_point);
                
                if (max_class_score > conf_threshold_) {
                    float x = data[0];
                    float y = data[1];
                    float w = data[2];
                    float h = data[3];
                    
                    int left = static_cast<int>((x - 0.5 * w) * x_scale);
                    int top = static_cast<int>((y - 0.5 * h) * y_scale);
                    int width = static_cast<int>(w * x_scale);
                    int height = static_cast<int>(h * y_scale);
                    
                    boxes.push_back(cv::Rect(left, top, width, height));
                    confidences.push_back(confidence);
                    class_ids.push_back(class_id_point.x);
                }
            }
            data += dimensions;
        }
    }
    
    // Apply NMS
    std::vector<int> indices = nms(boxes, confidences, nms_threshold_);
    
    // Create detection results
    int64_t timestamp = TimeClock::now();
    for (int idx : indices) {
        DetectionResult result;
        result.class_id = class_ids[idx];
        result.class_name = (class_ids[idx] < class_names_.size()) ? 
                           class_names_[class_ids[idx]] : "unknown";
        result.confidence = confidences[idx];
        result.bbox = boxes[idx];
        result.timestamp = timestamp;
        result.stream_id = stream_id;
        
        results.push_back(result);
    }
    
    return results;
}

std::vector<std::string> YOLODetector::getOutputLayerNames() {
    std::vector<std::string> names;
    std::vector<int> out_layers = net_.getUnconnectedOutLayers();
    std::vector<std::string> layers_names = net_.getLayerNames();
    
    names.resize(out_layers.size());
    for (size_t i = 0; i < out_layers.size(); ++i) {
        names[i] = layers_names[out_layers[i] - 1];
    }
    
    return names;
}

bool YOLODetector::updateConfig(const AIConfig& config) {
    config_ = config;
    conf_threshold_ = config_.confidence_threshold;
    
    // Reload model if path changed
    if (config_.object_model_path != config.object_model_path) {
        return loadModel(config_.object_model_path);
    }
    
    return true;
}

} // namespace AI

#endif // ENABLE_AI
