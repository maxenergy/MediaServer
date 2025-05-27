#ifdef ENABLE_AI

#include "AIDetector.h"
#include "YOLODetector.h"
#include "FaceDetector.h"
#include "PlateDetector.h"
#include <algorithm>

namespace AI {

cv::Mat AIDetector::frameToMat(const FrameBuffer::Ptr& frame) {
    if (!frame || frame->size() == 0) {
        return cv::Mat();
    }
    
    // Assume the frame is in YUV420P format (common for video streams)
    // For now, we'll implement a simple conversion
    // In a real implementation, you'd need to handle different pixel formats
    
    // For demonstration, assume RGB24 format
    int width = 640;  // These should come from frame metadata
    int height = 480;
    
    cv::Mat mat(height, width, CV_8UC3);
    memcpy(mat.data, frame->data(), frame->size());
    
    return mat;
}

cv::Mat AIDetector::resizeImage(const cv::Mat& image, int target_width, int target_height) {
    if (image.empty()) {
        return cv::Mat();
    }
    
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(target_width, target_height));
    return resized;
}

std::vector<int> AIDetector::nms(const std::vector<cv::Rect>& boxes, 
                                const std::vector<float>& scores, 
                                float nms_threshold) {
    std::vector<int> indices;
    std::vector<std::pair<float, int>> score_index_pairs;
    
    // Create score-index pairs
    for (size_t i = 0; i < scores.size(); ++i) {
        score_index_pairs.push_back({scores[i], static_cast<int>(i)});
    }
    
    // Sort by score in descending order
    std::sort(score_index_pairs.begin(), score_index_pairs.end(), 
              [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
                  return a.first > b.first;
              });
    
    std::vector<bool> suppressed(boxes.size(), false);
    
    for (const auto& pair : score_index_pairs) {
        int idx = pair.second;
        if (suppressed[idx]) continue;
        
        indices.push_back(idx);
        
        // Suppress overlapping boxes
        for (size_t j = 0; j < boxes.size(); ++j) {
            if (suppressed[j] || j == idx) continue;
            
            // Calculate IoU
            cv::Rect intersection = boxes[idx] & boxes[j];
            float intersection_area = intersection.area();
            float union_area = boxes[idx].area() + boxes[j].area() - intersection_area;
            
            if (union_area > 0) {
                float iou = intersection_area / union_area;
                if (iou > nms_threshold) {
                    suppressed[j] = true;
                }
            }
        }
    }
    
    return indices;
}

std::shared_ptr<AIDetector> AIDetectorFactory::create(DetectorType type) {
    switch (type) {
        case YOLO_DETECTOR:
            return std::make_shared<YOLODetector>();
        case FACE_DETECTOR:
            return std::make_shared<FaceDetector>();
        case PLATE_DETECTOR:
            return std::make_shared<PlateDetector>();
        default:
            logError << "Unknown detector type: " << type;
            return nullptr;
    }
}

} // namespace AI

#endif // ENABLE_AI
