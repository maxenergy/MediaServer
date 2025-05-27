#ifdef ENABLE_AI

#include "FaceDetector.h"
#include "Log/Logger.h"
#include "Util/TimeClock.h"
#include <algorithm>
#include <cmath>

namespace AI {

FaceDetector::FaceDetector() {
}

FaceDetector::~FaceDetector() {
}

bool FaceDetector::init(const AIConfig& config) {
    config_ = config;
    
    if (!config_.face_recognition_enabled) {
        logInfo << "Face recognition is disabled";
        return true;
    }
    
    detection_threshold_ = config_.confidence_threshold;
    
    // Load detection model
    if (!loadDetectionModel(config_.face_detection_model)) {
        logError << "Failed to load face detection model: " << config_.face_detection_model;
        return false;
    }
    
    // Load recognition model
    if (!loadRecognitionModel(config_.face_recognition_model)) {
        logError << "Failed to load face recognition model: " << config_.face_recognition_model;
        return false;
    }
    
    logInfo << "Face detector initialized successfully";
    return true;
}

bool FaceDetector::loadDetectionModel(const std::string& model_path) {
    try {
        // For now, use OpenCV's built-in face detector as fallback
        // In a real implementation, you'd load the RetinaFace model
        logInfo << "Loading face detection model: " << model_path;
        
        // Placeholder - would load actual model
        detection_model_loaded_ = true;
        return true;
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception while loading face detection model: " << e.what();
        return false;
    }
}

bool FaceDetector::loadRecognitionModel(const std::string& model_path) {
    try {
        logInfo << "Loading face recognition model: " << model_path;
        
        // Placeholder - would load actual ArcFace model
        recognition_model_loaded_ = true;
        return true;
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception while loading face recognition model: " << e.what();
        return false;
    }
}

std::vector<DetectionResult> FaceDetector::detect(const cv::Mat& frame, const std::string& stream_id) {
    std::vector<DetectionResult> results;
    
    if (!detection_model_loaded_ || !enabled_ || frame.empty()) {
        return results;
    }
    
    try {
        // Detect faces
        std::vector<cv::Rect> face_boxes = detectFaceBoxes(frame);
        
        int64_t timestamp = TimeClock::now();
        
        // Convert to DetectionResult format
        for (const auto& box : face_boxes) {
            DetectionResult result;
            result.class_id = 0; // Face class
            result.class_name = "face";
            result.confidence = 0.8f; // Placeholder confidence
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
        logError << "OpenCV exception during face detection: " << e.what();
    }
    
    return results;
}

std::vector<FaceResult> FaceDetector::detectFaces(const cv::Mat& frame, const std::string& stream_id) {
    std::vector<FaceResult> results;
    
    if (!detection_model_loaded_ || !enabled_ || frame.empty()) {
        return results;
    }
    
    try {
        // Detect faces
        std::vector<cv::Rect> face_boxes = detectFaceBoxes(frame);
        
        int64_t timestamp = TimeClock::now();
        
        // Process each detected face
        for (const auto& box : face_boxes) {
            FaceResult result;
            result.bbox = box;
            result.confidence = 0.8f; // Placeholder
            result.timestamp = timestamp;
            result.stream_id = stream_id;
            
            // Extract face region
            cv::Mat face_roi = frame(box);
            if (!face_roi.empty() && recognition_model_loaded_) {
                // Extract embedding
                result.embedding = extractFaceEmbedding(face_roi);
                
                // Find best match in database
                auto match = findBestMatch(result.embedding);
                if (match.second > recognition_threshold_) {
                    result.person_id = match.first;
                    // Get person name from database
                    auto it = face_database_.find(match.first);
                    if (it != face_database_.end()) {
                        result.person_name = it->second.name;
                    }
                }
            }
            
            results.push_back(result);
        }
        
        // Call face callback if set
        if (face_callback_ && !results.empty()) {
            face_callback_(results);
        }
        
    } catch (const cv::Exception& e) {
        logError << "OpenCV exception during face recognition: " << e.what();
    }
    
    return results;
}

std::vector<cv::Rect> FaceDetector::detectFaceBoxes(const cv::Mat& image) {
    std::vector<cv::Rect> faces;
    
    // Placeholder implementation using OpenCV's Haar cascades
    // In a real implementation, you'd use RetinaFace or similar
    static cv::CascadeClassifier face_cascade;
    static bool cascade_loaded = false;
    
    if (!cascade_loaded) {
        // Try to load Haar cascade
        if (face_cascade.load(cv::samples::findFile("haarcascade_frontalface_alt.xml"))) {
            cascade_loaded = true;
        } else {
            logWarn << "Could not load face cascade, using placeholder detection";
            return faces;
        }
    }
    
    if (cascade_loaded) {
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));
    }
    
    return faces;
}

std::vector<float> FaceDetector::extractFaceEmbedding(const cv::Mat& face_image) {
    // Placeholder implementation
    // In a real implementation, you'd use ArcFace or similar model
    std::vector<float> embedding(512, 0.0f); // ArcFace typically outputs 512-dim embeddings
    
    // Generate a simple placeholder embedding based on image properties
    cv::Scalar mean = cv::mean(face_image);
    for (int i = 0; i < 512; ++i) {
        embedding[i] = static_cast<float>(mean[i % 3]) / 255.0f + 
                      static_cast<float>(i) / 512.0f * 0.1f;
    }
    
    return embedding;
}

std::pair<std::string, float> FaceDetector::findBestMatch(const std::vector<float>& embedding) {
    std::string best_person_id;
    float best_similarity = 0.0f;
    
    for (const auto& person : face_database_) {
        for (const auto& stored_embedding : person.second.embeddings) {
            float similarity = cosineSimilarity(embedding, stored_embedding);
            if (similarity > best_similarity) {
                best_similarity = similarity;
                best_person_id = person.first;
            }
        }
    }
    
    return {best_person_id, best_similarity};
}

float FaceDetector::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size()) {
        return 0.0f;
    }
    
    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    
    for (size_t i = 0; i < a.size(); ++i) {
        dot_product += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    
    if (norm_a == 0.0f || norm_b == 0.0f) {
        return 0.0f;
    }
    
    return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

bool FaceDetector::addPerson(const std::string& person_id, const std::string& person_name, 
                            const std::vector<cv::Mat>& face_images) {
    if (person_id.empty() || face_images.empty()) {
        return false;
    }
    
    PersonInfo person_info;
    person_info.name = person_name;
    
    // Extract embeddings for all face images
    for (const auto& face_image : face_images) {
        if (!face_image.empty()) {
            auto embedding = extractFaceEmbedding(face_image);
            person_info.embeddings.push_back(embedding);
        }
    }
    
    if (person_info.embeddings.empty()) {
        logError << "No valid face embeddings extracted for person: " << person_id;
        return false;
    }
    
    face_database_[person_id] = person_info;
    logInfo << "Added person to face database: " << person_id << " (" << person_name << ") with " 
            << person_info.embeddings.size() << " face embeddings";
    
    return true;
}

bool FaceDetector::removePerson(const std::string& person_id) {
    auto it = face_database_.find(person_id);
    if (it != face_database_.end()) {
        face_database_.erase(it);
        logInfo << "Removed person from face database: " << person_id;
        return true;
    }
    
    logWarn << "Person not found in face database: " << person_id;
    return false;
}

std::vector<std::pair<std::string, std::string>> FaceDetector::getRegisteredPersons() const {
    std::vector<std::pair<std::string, std::string>> persons;
    
    for (const auto& person : face_database_) {
        persons.push_back({person.first, person.second.name});
    }
    
    return persons;
}

bool FaceDetector::updateConfig(const AIConfig& config) {
    config_ = config;
    detection_threshold_ = config_.confidence_threshold;
    
    // Reload models if paths changed
    if (config_.face_detection_model != config.face_detection_model) {
        return loadDetectionModel(config_.face_detection_model);
    }
    
    if (config_.face_recognition_model != config.face_recognition_model) {
        return loadRecognitionModel(config_.face_recognition_model);
    }
    
    return true;
}

cv::Mat FaceDetector::preprocessFaceForRecognition(const cv::Mat& face) {
    cv::Mat processed;
    
    // Resize to standard size (e.g., 112x112 for ArcFace)
    cv::resize(face, processed, cv::Size(112, 112));
    
    // Normalize
    processed.convertTo(processed, CV_32F, 1.0/255.0);
    
    return processed;
}

cv::Mat FaceDetector::alignFace(const cv::Mat& face) {
    // Placeholder for face alignment
    // In a real implementation, you'd detect facial landmarks and align the face
    return face;
}

} // namespace AI

#endif // ENABLE_AI
