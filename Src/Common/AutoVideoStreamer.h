#ifndef AutoVideoStreamer_h
#define AutoVideoStreamer_h

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>

#include "EventPoller/EventLoop.h"
#include "MediaSource.h"
#include "HookManager.h"

using namespace std;

// 文件事件类型
enum class FileEventType {
    FILE_CREATED,
    FILE_DELETED,
    FILE_MOVED_FROM,
    FILE_MOVED_TO,
    FILE_MODIFIED
};

// 文件事件结构
struct FileEvent {
    FileEventType type;
    string filePath;
    string fileName;
    uint32_t cookie;        // 用于关联MOVE事件
    uint64_t timestamp;     // 事件时间戳
};

// 待处理文件信息（用于防抖）
struct PendingFileInfo {
    string filePath;
    string fileName;
    uint64_t lastEventTime;
    bool isStable;
};

struct VideoStreamInfo {
    string filePath;        // 视频文件完整路径
    string fileName;        // 文件名（不含扩展名）
    string rtspUrl;         // RTSP推流地址
    MediaSource::Ptr source; // 媒体源指针
    bool isActive;          // 是否激活
};

class AutoVideoStreamer : public HookBase
{
public:
    using Ptr = shared_ptr<AutoVideoStreamer>;
    using Wptr = weak_ptr<AutoVideoStreamer>;

    AutoVideoStreamer();
    ~AutoVideoStreamer();

public:
    // 单例模式
    static AutoVideoStreamer::Ptr& instance();

    // 初始化自动推流器
    bool init();

    // 启动自动推流
    bool start();

    // 停止自动推流
    void stop();

    // 扫描视频文件并创建推流
    void scanAndCreateStreams();

    // 获取本机IP地址
    string getLocalIpAddress();

    // 获取RTSP端口
    int getRtspPort();

    // 生成RTSP URL
    string generateRtspUrl(const string& fileName);

    // 为视频文件创建媒体源
    bool createMediaSourceForVideo(const string& filePath, const string& fileName);

    // 获取所有推流信息
    vector<VideoStreamInfo> getAllStreamInfo();

    // 检查文件是否为支持的视频格式
    bool isSupportedVideoFile(const string& filePath);

    // 从文件路径提取文件名（不含扩展名）
    string extractFileName(const string& filePath);

    // 文件监控相关方法
    bool startFileMonitor();
    void stopFileMonitor();
    bool isFileMonitorEnabled() const { return _enableFileMonitor; }

    // HookBase interface implementation
    void onStreamStatus(const StreamStatusInfo& info) override {}
    void onStreamHeartbeat(const StreamHeartbeatInfo& info) override {}
    void onPublish(const PublishInfo& info, const function<void(const PublishResponse& rsp)>& cb) override {
        PublishResponse rsp;
        rsp.authResult = true;
        cb(rsp);
    }
    void onPlay(const PlayInfo& info, const function<void(const PlayResponse& rsp)>& cb) override {
        PlayResponse rsp;
        rsp.authResult = true;
        cb(rsp);
    }
    void onPlayer(const PlayerInfo& info) override {}
    void onNonePlayer(const string& protocol, const string& uri, const string& vhost, const string& type) override {}
    void onKeepAlive(const ServerInfo& info) override {}
    void onRegisterServer(const RegisterServerInfo& info) override {}
    void onRecord(const OnRecordInfo& info) override {}
    void onStreamNotFound(const OnStreamNotFoundInfo& info, const function<void(const OnStreamNotFoundResponse& rsp)>& cb) override;

private:
    // 递归扫描目录
    void scanDirectory(const string& dirPath, vector<string>& videoFiles);

    // 清理无效的推流
    void cleanupInvalidStreams();



    // 文件监控相关私有方法
    void fileMonitorThread();
    void processFileEvents();
    void handleFileEvent(const FileEvent& event);
    void addWatchDirectory(const string& dirPath);
    void removeWatchDirectory(const string& dirPath);
    bool setupInotify();
    void cleanupInotify();
    void processStabilityCheck();
    void onFileCreated(const string& filePath, const string& fileName);
    void onFileDeleted(const string& filePath, const string& fileName);
    void onFileMovedFrom(const string& filePath, const string& fileName, uint32_t cookie);
    void onFileMovedTo(const string& filePath, const string& fileName, uint32_t cookie);

private:
    bool _initialized = false;
    bool _running = false;
    string _videoDir;
    bool _scanSubdirectories = true;
    bool _enabled = true;

    // 文件监控相关配置
    bool _enableFileMonitor = true;
    uint64_t _fileStabilityDelayMs = 2000;
    int _maxConcurrentFileOps = 5;

    // 文件监控相关状态
    atomic<bool> _monitorRunning{false};
    thread _monitorThread;
    int _inotifyFd = -1;
    unordered_map<int, string> _watchDescriptors; // wd -> dirPath
    unordered_map<string, int> _directoryWatches; // dirPath -> wd

    // 事件处理
    mutex _eventMtx;
    queue<FileEvent> _eventQueue;
    condition_variable _eventCv;

    // 防抖处理
    mutex _pendingMtx;
    unordered_map<string, PendingFileInfo> _pendingFiles; // filePath -> info
    unordered_map<uint32_t, string> _moveFromFiles; // cookie -> filePath

    // 并发控制
    atomic<int> _currentFileOps{0};

    mutex _mtx;
    unordered_map<string, VideoStreamInfo> _streamInfoMap; // key: fileName

    EventLoop::Ptr _loop;
};

#endif //AutoVideoStreamer_h
