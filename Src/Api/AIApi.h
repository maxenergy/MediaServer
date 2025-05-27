#ifndef AI_API_H
#define AI_API_H

#ifdef ENABLE_AI

#include "Http/HttpParser.h"
#include "Common/UrlParser.h"
#include "Http/HttpResponse.h"
#include <functional>

class AIApi {
public:
    // Initialize AI API endpoints
    static void initApi();
    
    // AI detection control endpoints
    static void startDetection(const HttpParser& parser, const UrlParser& urlParser, 
                              const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void stopDetection(const HttpParser& parser, const UrlParser& urlParser, 
                             const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void getDetectionStatus(const HttpParser& parser, const UrlParser& urlParser, 
                                  const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    // AI configuration endpoints
    static void getAIConfig(const HttpParser& parser, const UrlParser& urlParser, 
                           const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void updateAIConfig(const HttpParser& parser, const UrlParser& urlParser, 
                              const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    // Stream-specific AI control
    static void enableStreamAI(const HttpParser& parser, const UrlParser& urlParser, 
                              const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void disableStreamAI(const HttpParser& parser, const UrlParser& urlParser, 
                               const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void getStreamAIStatus(const HttpParser& parser, const UrlParser& urlParser, 
                                 const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    // Model management endpoints
    static void listModels(const HttpParser& parser, const UrlParser& urlParser, 
                          const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void loadModel(const HttpParser& parser, const UrlParser& urlParser, 
                         const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void unloadModel(const HttpParser& parser, const UrlParser& urlParser, 
                           const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    // Face database management endpoints
    static void addPerson(const HttpParser& parser, const UrlParser& urlParser, 
                         const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void removePerson(const HttpParser& parser, const UrlParser& urlParser, 
                            const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void listPersons(const HttpParser& parser, const UrlParser& urlParser, 
                           const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    // Statistics and monitoring endpoints
    static void getAIStats(const HttpParser& parser, const UrlParser& urlParser, 
                          const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void resetAIStats(const HttpParser& parser, const UrlParser& urlParser, 
                            const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    // Detection results endpoints
    static void getRecentDetections(const HttpParser& parser, const UrlParser& urlParser, 
                                   const std::function<void(HttpResponse& rsp)>& rspFunc);
    
    static void getDetectionHistory(const HttpParser& parser, const UrlParser& urlParser, 
                                   const std::function<void(HttpResponse& rsp)>& rspFunc);

private:
    // Helper functions
    static bool validateStreamId(const std::string& stream_id);
    static bool validatePersonId(const std::string& person_id);
    static std::string generateErrorResponse(const std::string& error_msg);
    static std::string generateSuccessResponse(const nlohmann::json& data = nlohmann::json::object());
};

#endif // ENABLE_AI

#endif // AI_API_H
