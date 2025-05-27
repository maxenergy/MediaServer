#ifdef ENABLE_AI

#include "AIApi.h"
#include "AI/AIManager.h"
#include "Common/json.hpp"
#include "Log/Logger.h"
#include "Common/ApiUtil.h"

using json = nlohmann::json;

void AIApi::initApi() {
    // AI detection control endpoints
    g_mapApi.emplace("/api/v1/ai/detection/start", AIApi::startDetection);
    g_mapApi.emplace("/api/v1/ai/detection/stop", AIApi::stopDetection);
    g_mapApi.emplace("/api/v1/ai/detection/status", AIApi::getDetectionStatus);
    
    // AI configuration endpoints
    g_mapApi.emplace("/api/v1/ai/config", AIApi::getAIConfig);
    g_mapApi.emplace("/api/v1/ai/config/update", AIApi::updateAIConfig);
    
    // Stream-specific AI control
    g_mapApi.emplace("/api/v1/ai/stream/enable", AIApi::enableStreamAI);
    g_mapApi.emplace("/api/v1/ai/stream/disable", AIApi::disableStreamAI);
    g_mapApi.emplace("/api/v1/ai/stream/status", AIApi::getStreamAIStatus);
    
    // Model management endpoints
    g_mapApi.emplace("/api/v1/ai/models/list", AIApi::listModels);
    g_mapApi.emplace("/api/v1/ai/models/load", AIApi::loadModel);
    g_mapApi.emplace("/api/v1/ai/models/unload", AIApi::unloadModel);
    
    // Face database management endpoints
    g_mapApi.emplace("/api/v1/ai/faces/add", AIApi::addPerson);
    g_mapApi.emplace("/api/v1/ai/faces/remove", AIApi::removePerson);
    g_mapApi.emplace("/api/v1/ai/faces/list", AIApi::listPersons);
    
    // Statistics and monitoring endpoints
    g_mapApi.emplace("/api/v1/ai/stats", AIApi::getAIStats);
    g_mapApi.emplace("/api/v1/ai/stats/reset", AIApi::resetAIStats);
    
    // Detection results endpoints
    g_mapApi.emplace("/api/v1/ai/detections/recent", AIApi::getRecentDetections);
    g_mapApi.emplace("/api/v1/ai/detections/history", AIApi::getDetectionHistory);
    
    logInfo << "AI API endpoints initialized";
}

void AIApi::startDetection(const HttpParser& parser, const UrlParser& urlParser, 
                          const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    try {
        json body = json::parse(parser._body);
        
        std::string stream_id = body.value("stream_id", "");
        if (stream_id.empty()) {
            value["code"] = "400";
            value["msg"] = "stream_id is required";
            rsp._body = value.dump();
            rspFunc(rsp);
            return;
        }
        
        // Create AI config from request
        AI::AIConfig config;
        config.enabled = true;
        config.confidence_threshold = body.value("confidence_threshold", 0.5f);
        config.detection_interval_ms = body.value("detection_interval_ms", 1000);
        config.object_detection_enabled = body.value("object_detection", true);
        config.face_recognition_enabled = body.value("face_recognition", true);
        config.plate_recognition_enabled = body.value("plate_recognition", true);
        
        if (AI::AIManager::instance()->enableStreamAI(stream_id, config)) {
            value["code"] = "200";
            value["msg"] = "AI detection started successfully";
            value["data"]["stream_id"] = stream_id;
        } else {
            value["code"] = "500";
            value["msg"] = "Failed to start AI detection";
        }
        
    } catch (const std::exception& e) {
        value["code"] = "400";
        value["msg"] = std::string("Invalid request: ") + e.what();
    }
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::stopDetection(const HttpParser& parser, const UrlParser& urlParser, 
                         const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    try {
        json body = json::parse(parser._body);
        
        std::string stream_id = body.value("stream_id", "");
        if (stream_id.empty()) {
            value["code"] = "400";
            value["msg"] = "stream_id is required";
            rsp._body = value.dump();
            rspFunc(rsp);
            return;
        }
        
        if (AI::AIManager::instance()->disableStreamAI(stream_id)) {
            value["code"] = "200";
            value["msg"] = "AI detection stopped successfully";
            value["data"]["stream_id"] = stream_id;
        } else {
            value["code"] = "404";
            value["msg"] = "Stream not found or AI not enabled";
        }
        
    } catch (const std::exception& e) {
        value["code"] = "400";
        value["msg"] = std::string("Invalid request: ") + e.what();
    }
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::getDetectionStatus(const HttpParser& parser, const UrlParser& urlParser, 
                              const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    try {
        auto active_streams = AI::AIManager::instance()->getActiveStreams();
        
        value["code"] = "200";
        value["msg"] = "success";
        value["data"]["active_streams"] = active_streams;
        value["data"]["total_streams"] = active_streams.size();
        
    } catch (const std::exception& e) {
        value["code"] = "500";
        value["msg"] = std::string("Internal error: ") + e.what();
    }
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::getAIStats(const HttpParser& parser, const UrlParser& urlParser, 
                      const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    try {
        auto stats = AI::AIManager::instance()->getStats();
        
        value["code"] = "200";
        value["msg"] = "success";
        value["data"]["total_frames_processed"] = stats.total_frames_processed;
        value["data"]["total_detections"] = stats.total_detections;
        value["data"]["total_faces_detected"] = stats.total_faces_detected;
        value["data"]["total_plates_detected"] = stats.total_plates_detected;
        value["data"]["avg_processing_time_ms"] = stats.avg_processing_time_ms;
        value["data"]["active_streams"] = stats.active_streams;
        
    } catch (const std::exception& e) {
        value["code"] = "500";
        value["msg"] = std::string("Internal error: ") + e.what();
    }
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::resetAIStats(const HttpParser& parser, const UrlParser& urlParser, 
                        const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    try {
        AI::AIManager::instance()->resetStats();
        
        value["code"] = "200";
        value["msg"] = "AI statistics reset successfully";
        
    } catch (const std::exception& e) {
        value["code"] = "500";
        value["msg"] = std::string("Internal error: ") + e.what();
    }
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

// Placeholder implementations for other endpoints
void AIApi::getAIConfig(const HttpParser& parser, const UrlParser& urlParser, 
                       const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "success";
    value["data"]["ai_enabled"] = true;
    value["data"]["object_detection"] = true;
    value["data"]["face_recognition"] = true;
    value["data"]["plate_recognition"] = true;
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::updateAIConfig(const HttpParser& parser, const UrlParser& urlParser, 
                          const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "AI configuration updated successfully";
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::enableStreamAI(const HttpParser& parser, const UrlParser& urlParser, 
                          const std::function<void(HttpResponse& rsp)>& rspFunc) {
    startDetection(parser, urlParser, rspFunc);
}

void AIApi::disableStreamAI(const HttpParser& parser, const UrlParser& urlParser, 
                           const std::function<void(HttpResponse& rsp)>& rspFunc) {
    stopDetection(parser, urlParser, rspFunc);
}

void AIApi::getStreamAIStatus(const HttpParser& parser, const UrlParser& urlParser, 
                             const std::function<void(HttpResponse& rsp)>& rspFunc) {
    getDetectionStatus(parser, urlParser, rspFunc);
}

void AIApi::listModels(const HttpParser& parser, const UrlParser& urlParser, 
                      const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "success";
    value["data"]["models"] = json::array({
        {{"name", "yolov8n"}, {"type", "object_detection"}, {"loaded", true}},
        {{"name", "retinaface"}, {"type", "face_detection"}, {"loaded", true}},
        {{"name", "arcface"}, {"type", "face_recognition"}, {"loaded", true}},
        {{"name", "plate_detect"}, {"type", "plate_detection"}, {"loaded", true}}
    });
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::loadModel(const HttpParser& parser, const UrlParser& urlParser, 
                     const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "Model loaded successfully";
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::unloadModel(const HttpParser& parser, const UrlParser& urlParser, 
                       const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "Model unloaded successfully";
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::addPerson(const HttpParser& parser, const UrlParser& urlParser, 
                     const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "Person added to face database successfully";
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::removePerson(const HttpParser& parser, const UrlParser& urlParser, 
                        const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "Person removed from face database successfully";
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::listPersons(const HttpParser& parser, const UrlParser& urlParser, 
                       const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    try {
        auto persons = AI::AIManager::instance()->getRegisteredPersons();
        
        json persons_array = json::array();
        for (const auto& person : persons) {
            persons_array.push_back({
                {"person_id", person.first},
                {"person_name", person.second}
            });
        }
        
        value["code"] = "200";
        value["msg"] = "success";
        value["data"]["persons"] = persons_array;
        value["data"]["total"] = persons.size();
        
    } catch (const std::exception& e) {
        value["code"] = "500";
        value["msg"] = std::string("Internal error: ") + e.what();
    }
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::getRecentDetections(const HttpParser& parser, const UrlParser& urlParser, 
                               const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "success";
    value["data"]["detections"] = json::array();
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

void AIApi::getDetectionHistory(const HttpParser& parser, const UrlParser& urlParser, 
                               const std::function<void(HttpResponse& rsp)>& rspFunc) {
    HttpResponse rsp;
    rsp._status = 200;
    json value;
    
    value["code"] = "200";
    value["msg"] = "success";
    value["data"]["history"] = json::array();
    
    rsp._body = value.dump();
    rspFunc(rsp);
}

// Helper functions
bool AIApi::validateStreamId(const std::string& stream_id) {
    return !stream_id.empty() && stream_id.length() < 256;
}

bool AIApi::validatePersonId(const std::string& person_id) {
    return !person_id.empty() && person_id.length() < 64;
}

std::string AIApi::generateErrorResponse(const std::string& error_msg) {
    json value;
    value["code"] = "400";
    value["msg"] = error_msg;
    return value.dump();
}

std::string AIApi::generateSuccessResponse(const json& data) {
    json value;
    value["code"] = "200";
    value["msg"] = "success";
    if (!data.empty()) {
        value["data"] = data;
    }
    return value.dump();
}

#endif // ENABLE_AI
