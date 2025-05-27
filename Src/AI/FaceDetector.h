#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#ifdef ENABLE_AI

#include "AIDetector.h"
#include <opencv2/dnn.hpp>
#include <unordered_map>

namespace AI {

class FaceDetector : public AIDetector {
public:
    FaceDetector();
    virtual ~FaceDetector();
    
    // Initialize face detector
    bool init(const AIConfig& config) override;
    
    // Detect faces in frame (returns as DetectionResult for compatibility)
    std::vector<DetectionResult> detect(const cv::Mat& frame, const std::string& stream_id) override;
    
    // Face-specific detection method
    std::vector<FaceResult> detectFaces(const cv::Mat& frame, const std::string& stream_id);
    
    // Set face recognition callback
    void setFaceCallback(FaceCallback callback) { face_callback_ = callback; }
    
    // Add a person to the face database
    bool addPerson(const std::string& person_id, const std::string& person_name, 
                   const std::vector<cv::Mat>& face_images);
    
    // Remove a person from the face database
    bool removePerson(const std::string& person_id);
    
    // Get all registered persons
    std::vector<std::pair<std::string, std::string>> getRegisteredPersons() const;
    
    // Update configuration
    bool updateConfig(const AIConfig& config) override;

private:
    cv::dnn::Net detection_net_;
    cv::dnn::Net recognition_net_;
    bool detection_model_loaded_ = false;
    bool recognition_model_loaded_ = false;
    
    FaceCallback face_callback_;
    
    // Face database: person_id -> {name, embeddings}
    struct PersonInfo {
        std::string name;
        std::vector<std::vector<float>> embeddings;
    };
    std::unordered_map<std::string, PersonInfo> face_database_;
    
    // Detection parameters
    float detection_threshold_ = 0.7f;
    float recognition_threshold_ = 0.6f;
    int input_width_ = 640;
    int input_height_ = 640;
    
    // Load face detection model
    bool loadDetectionModel(const std::string& model_path);
    
    // Load face recognition model
    bool loadRecognitionModel(const std::string& model_path);
    
    // Detect faces using the detection model
    std::vector<cv::Rect> detectFaceBoxes(const cv::Mat& image);
    
    // Extract face embedding using recognition model
    std::vector<float> extractFaceEmbedding(const cv::Mat& face_image);
    
    // Find best matching person for a face embedding
    std::pair<std::string, float> findBestMatch(const std::vector<float>& embedding);
    
    // Calculate cosine similarity between two embeddings
    float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
    
    // Preprocess face image for recognition
    cv::Mat preprocessFaceForRecognition(const cv::Mat& face);
    
    // Align face (simple version)
    cv::Mat alignFace(const cv::Mat& face);
};

} // namespace AI

#endif // ENABLE_AI

#endif // FACE_DETECTOR_H
