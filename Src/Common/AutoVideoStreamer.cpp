#include "AutoVideoStreamer.h"
#include "Config.h"
#include "Logger.h"
#include "Util/File.h"
#include "Util/String.h"
#include "EventLoopPool.h"
#include "FrameMediaSource.h"
#include "RecordReaderBase.h"
#include "UrlParser.h"
#include "Define.h"

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <cstring>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <cstdlib>

using namespace std;

AutoVideoStreamer::AutoVideoStreamer()
{
    _loop = EventLoop::getCurrentLoop();
}

AutoVideoStreamer::~AutoVideoStreamer()
{
    stop();
}

AutoVideoStreamer::Ptr& AutoVideoStreamer::instance()
{
    static AutoVideoStreamer::Ptr instance = make_shared<AutoVideoStreamer>();
    return instance;
}

bool AutoVideoStreamer::init()
{
    if (_initialized) {
        return true;
    }

    // 从配置文件读取设置
    _enabled = Config::instance()->get("AutoVideoStreamer", "enable", "", "", "true").as<bool>();
    if (!_enabled) {
        logInfo << "AutoVideoStreamer is disabled in config";
        return false;
    }

    _videoDir = Config::instance()->get("AutoVideoStreamer", "videoDir", "", "", "./Videos").as<string>();
    _scanSubdirectories = Config::instance()->get("AutoVideoStreamer", "scanSubdirectories", "", "", "true").as<bool>();
    _enableFileMonitor = Config::instance()->get("AutoVideoStreamer", "enableFileMonitor", "", "", "true").as<bool>();
    _fileStabilityDelayMs = Config::instance()->get("AutoVideoStreamer", "fileStabilityDelayMs", "", "", "2000").as<uint64_t>();
    _maxConcurrentFileOps = Config::instance()->get("AutoVideoStreamer", "maxConcurrentFileOps", "", "", "5").as<int>();

    logInfo << "AutoVideoStreamer initialized - videoDir: " << _videoDir
            << ", scanSubdirectories: " << _scanSubdirectories
            << ", enableFileMonitor: " << _enableFileMonitor
            << ", fileStabilityDelayMs: " << _fileStabilityDelayMs
            << ", maxConcurrentFileOps: " << _maxConcurrentFileOps;

    // 注册为主要的MediaHook，替换默认的MediaHook
    HookManager::instance()->addHook(MEDIA_HOOK, shared_from_this());
    logInfo << "AutoVideoStreamer registered as primary MediaHook";

    _initialized = true;
    return true;
}

bool AutoVideoStreamer::start()
{
    if (!_initialized) {
        logError << "AutoVideoStreamer not initialized";
        return false;
    }

    if (_running) {
        logWarn << "AutoVideoStreamer already running";
        return true;
    }

    // 检查视频目录是否存在
    if (!File::isDir(_videoDir.c_str())) {
        logWarn << "Video directory does not exist: " << _videoDir;
        // 尝试创建目录
        if (!File::createDir(_videoDir.c_str(), 0755)) {
            logError << "Failed to create video directory: " << _videoDir;
            return false;
        }
        logInfo << "Created video directory: " << _videoDir;
    }

    _running = true;

    // 扫描并创建推流
    scanAndCreateStreams();

    // 启动文件监控
    if (_enableFileMonitor) {
        if (startFileMonitor()) {
            logInfo << "File monitor started successfully";
        } else {
            logWarn << "Failed to start file monitor, continuing without monitoring";
        }
    }

    logInfo << "AutoVideoStreamer started successfully";
    return true;
}

void AutoVideoStreamer::stop()
{
    if (!_running) {
        return;
    }

    _running = false;

    // 停止文件监控
    if (_enableFileMonitor) {
        stopFileMonitor();
    }

    lock_guard<mutex> lck(_mtx);
    _streamInfoMap.clear();

    logInfo << "AutoVideoStreamer stopped";
}

void AutoVideoStreamer::scanAndCreateStreams()
{
    if (!_running) {
        return;
    }

    vector<string> videoFiles;
    scanDirectory(_videoDir, videoFiles);

    logInfo << "Found " << videoFiles.size() << " video files";

    lock_guard<mutex> lck(_mtx);
    
    // 清理现有的推流信息
    _streamInfoMap.clear();

    // 为每个视频文件创建推流
    for (const auto& filePath : videoFiles) {
        string fileName = extractFileName(filePath);
        if (fileName.empty()) {
            continue;
        }

        if (createMediaSourceForVideo(filePath, fileName)) {
            logInfo << "Created stream for: " << fileName << " -> " << generateRtspUrl(fileName);
        } else {
            logWarn << "Failed to create stream for: " << filePath;
        }
    }

    logInfo << "AutoVideoStreamer created " << _streamInfoMap.size() << " streams";
}

string AutoVideoStreamer::getLocalIpAddress()
{
    struct ifaddrs *ifaddrs_ptr = nullptr;
    struct ifaddrs *ifa = nullptr;
    string localIp = "127.0.0.1"; // 默认值

    if (getifaddrs(&ifaddrs_ptr) == -1) {
        logWarn << "Failed to get network interfaces";
        return localIp;
    }

    for (ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        // 只处理IPv4地址
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
            char ip_str[INET_ADDRSTRLEN];
            
            if (inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN) != nullptr) {
                string ip(ip_str);
                
                // 跳过回环地址和无效地址
                if (ip != "127.0.0.1" && ip != "0.0.0.0" && !startWith(ip, "169.254")) {
                    localIp = ip;
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddrs_ptr);
    return localIp;
}

int AutoVideoStreamer::getRtspPort()
{
    // 从配置中读取RTSP端口，如果失败则使用默认端口2554（避免需要root权限的554端口）
    try {
        int port = Config::instance()->get("Rtsp", "Server", "Server1", "port");
        if (port == 554) {
            // 如果配置的是554端口但没有root权限，使用2554端口
            return 2554;
        }
        return port;
    } catch (...) {
        logWarn << "Failed to read RTSP port from config, using default 2554";
        return 2554;
    }
}

string AutoVideoStreamer::generateRtspUrl(const string& fileName)
{
    // 获取本机IP和RTSP端口
    string localIp = getLocalIpAddress();
    int rtspPort = getRtspPort();

    return "rtsp://" + localIp + ":" + to_string(rtspPort) + "/stream/" + fileName;
}

bool AutoVideoStreamer::createMediaSourceForVideo(const string& filePath, const string& fileName)
{
    try {
        logDebug << "Creating media source for file: " << filePath << ", fileName: " << fileName;

        // 确保使用绝对路径
        string absolutePath = filePath;
        if (filePath[0] != '/') {
            // 如果是相对路径，转换为绝对路径
            char* cwd = getcwd(nullptr, 0);
            if (cwd) {
                absolutePath = string(cwd) + "/" + filePath;
                free(cwd);
            }
        }

        logDebug << "Using absolute path: " << absolutePath;

        // 构造符合RecordReaderMp4期望格式的URI
        // 格式：/file/vodId/relativePath/loopCount
        // RecordReaderMp4会去掉前两层目录，然后从Record.rootPath开始查找文件
        string uri = "/file/vod/Videos/" + fileName + ".mp4/1";  // 1表示播放一次

        VideoStreamInfo streamInfo;
        streamInfo.filePath = absolutePath;
        streamInfo.fileName = fileName;

        logDebug << "About to generate RTSP URL for: " << fileName;
        streamInfo.rtspUrl = generateRtspUrl(fileName);
        logDebug << "Generated RTSP URL: " << streamInfo.rtspUrl;

        streamInfo.isActive = false;

        // 恢复MediaSource::loadFromFile调用，使用正确的URI格式
        logInfo << "Creating media source for: " << fileName << " with URI: " << uri;

        // 使用文件URI创建媒体源，这样RecordReader能找到正确的文件
        MediaSource::loadFromFile(uri, DEFAULT_VHOST, PROTOCOL_RTSP, DEFAULT_TYPE,
            [this, fileName, streamInfo](const MediaSource::Ptr& src) mutable {
                if (src) {
                    streamInfo.source = src;
                    streamInfo.isActive = true;

                    lock_guard<mutex> lck(_mtx);
                    _streamInfoMap[fileName] = streamInfo;

                    logInfo << "Successfully created media source for: " << fileName;
                } else {
                    logError << "Failed to create media source for: " << fileName;
                }
            },
            [fileName]() -> MediaSource::Ptr {
                UrlParser urlParser;
                urlParser.protocol_ = PROTOCOL_RTSP;
                urlParser.path_ = "/stream/" + fileName;  // RTSP路径使用/stream/格式
                urlParser.vhost_ = DEFAULT_VHOST;
                urlParser.type_ = DEFAULT_TYPE;
                return make_shared<FrameMediaSource>(urlParser, nullptr);
            },
            this
        );

        return true;
    } catch (const exception& e) {
        logError << "Exception creating media source for " << fileName << ": " << e.what();
        return false;
    }
}

void AutoVideoStreamer::onStreamNotFound(const OnStreamNotFoundInfo& info, const function<void(const OnStreamNotFoundResponse& rsp)>& cb) {
    logError << "AutoVideoStreamer::onStreamNotFound called for URI: " << info.uri;

    try {
        OnStreamNotFoundResponse response;

        // 检查是否是我们管理的 /stream/ 路径
        if (info.uri.find("/stream/") == 0) {
            // 提取文件名
            string fileName = info.uri.substr(8); // 去掉 "/stream/" 前缀
            logInfo << "Extracted fileName: " << fileName;

            // 检查是否是我们管理的文件
            lock_guard<mutex> lock(_mtx);
            auto it = _streamInfoMap.find(fileName);
            if (it != _streamInfoMap.end()) {
                // 验证文件是否仍然存在
                string filePath = it->second.filePath;
                if (access(filePath.c_str(), F_OK) == 0) {
                    // 构建对应的文件URI，使用1表示播放一次
                    string fileUri = "/file/vod/Videos/" + fileName + ".mp4/1";
                    logInfo << "Mapping RTSP path " << info.uri << " to file URI: " << fileUri;
                    logInfo << "File exists at: " << filePath;

                    // 返回文件URI作为pullUrl，让系统重新创建媒体源
                    response.pullUrl = fileUri;
                } else {
                    logWarn << "File no longer exists: " << filePath;
                    response.pullUrl = "";
                }
            } else {
                logWarn << "File not found in stream map: " << fileName;
                // 列出当前管理的所有文件
                logInfo << "Currently managed files:";
                for (const auto& pair : _streamInfoMap) {
                    logInfo << "  - " << pair.first << " -> " << pair.second.filePath;
                }
                response.pullUrl = "";
            }
        } else {
            // 不是我们管理的路径，返回空
            logInfo << "URI not managed by AutoVideoStreamer: " << info.uri;
            response.pullUrl = "";
        }

        logInfo << "Calling callback with pullUrl: '" << response.pullUrl << "'";
        if (cb) {
            cb(response);
            logInfo << "Callback completed successfully";
        } else {
            logError << "Callback function is null!";
        }
    } catch (const exception& e) {
        logError << "Exception in onStreamNotFound: " << e.what();
        OnStreamNotFoundResponse response;
        response.pullUrl = "";
        if (cb) {
            cb(response);
        }
    } catch (...) {
        logError << "Unknown exception in onStreamNotFound";
        OnStreamNotFoundResponse response;
        response.pullUrl = "";
        if (cb) {
            cb(response);
        }
    }
}

vector<VideoStreamInfo> AutoVideoStreamer::getAllStreamInfo()
{
    lock_guard<mutex> lck(_mtx);
    vector<VideoStreamInfo> result;
    
    for (const auto& pair : _streamInfoMap) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool AutoVideoStreamer::isSupportedVideoFile(const string& filePath)
{
    return endWithIgnoreCase(filePath, ".mp4");
}

string AutoVideoStreamer::extractFileName(const string& filePath)
{
    // 提取文件名（不含路径）
    size_t lastSlash = filePath.find_last_of("/\\");
    string fileName = (lastSlash != string::npos) ? filePath.substr(lastSlash + 1) : filePath;
    
    // 去掉扩展名
    size_t lastDot = fileName.find_last_of(".");
    if (lastDot != string::npos) {
        fileName = fileName.substr(0, lastDot);
    }
    
    return fileName;
}

void AutoVideoStreamer::scanDirectory(const string& dirPath, vector<string>& videoFiles)
{
    File::scanDir(dirPath, [this, &videoFiles](const string& path, bool isDir) -> bool {
        if (isDir) {
            if (_scanSubdirectories) {
                scanDirectory(path, videoFiles);
            }
        } else {
            if (isSupportedVideoFile(path)) {
                videoFiles.push_back(path);
            }
        }
        return true; // 继续扫描
    }, false); // 不自动进入子目录，我们手动控制
}

void AutoVideoStreamer::cleanupInvalidStreams()
{
    lock_guard<mutex> lck(_mtx);
    
    auto it = _streamInfoMap.begin();
    while (it != _streamInfoMap.end()) {
        if (!it->second.isActive || !File::isFile(it->second.filePath.c_str())) {
            logInfo << "Removing invalid stream: " << it->first;
            it = _streamInfoMap.erase(it);
        } else {
            ++it;
        }
    }
}

// ==================== 文件监控相关实现 ====================

bool AutoVideoStreamer::startFileMonitor()
{
    if (_monitorRunning.load()) {
        logWarn << "File monitor is already running";
        return true;
    }

    if (!setupInotify()) {
        logError << "Failed to setup inotify";
        return false;
    }

    // 添加主目录监控
    addWatchDirectory(_videoDir);

    // 如果启用子目录扫描，添加所有子目录监控
    if (_scanSubdirectories) {
        vector<string> subdirs;
        File::scanDir(_videoDir, [&subdirs](const string& path, bool isDir) -> bool {
            if (isDir) {
                subdirs.push_back(path);
            }
            return true;
        }, false);

        for (const auto& subdir : subdirs) {
            addWatchDirectory(subdir);
        }
    }

    _monitorRunning.store(true);
    _monitorThread = thread(&AutoVideoStreamer::fileMonitorThread, this);

    logInfo << "File monitor started, watching " << _directoryWatches.size() << " directories";
    return true;
}

void AutoVideoStreamer::stopFileMonitor()
{
    if (!_monitorRunning.load()) {
        return;
    }

    _monitorRunning.store(false);

    // 唤醒监控线程
    {
        lock_guard<mutex> lck(_eventMtx);
        _eventCv.notify_all();
    }

    if (_monitorThread.joinable()) {
        _monitorThread.join();
    }

    cleanupInotify();

    logInfo << "File monitor stopped";
}

bool AutoVideoStreamer::setupInotify()
{
    _inotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (_inotifyFd == -1) {
        logError << "Failed to initialize inotify: " << strerror(errno);
        return false;
    }

    logDebug << "Inotify initialized with fd: " << _inotifyFd;
    return true;
}

void AutoVideoStreamer::cleanupInotify()
{
    // 移除所有监控
    for (const auto& pair : _directoryWatches) {
        inotify_rm_watch(_inotifyFd, pair.second);
    }

    _watchDescriptors.clear();
    _directoryWatches.clear();

    if (_inotifyFd != -1) {
        close(_inotifyFd);
        _inotifyFd = -1;
    }

    // 清理待处理文件
    {
        lock_guard<mutex> lck(_pendingMtx);
        _pendingFiles.clear();
        _moveFromFiles.clear();
    }

    logDebug << "Inotify cleaned up";
}

void AutoVideoStreamer::addWatchDirectory(const string& dirPath)
{
    if (_inotifyFd == -1) {
        logError << "Inotify not initialized";
        return;
    }

    // 检查目录是否已经在监控中
    if (_directoryWatches.find(dirPath) != _directoryWatches.end()) {
        logDebug << "Directory already being watched: " << dirPath;
        return;
    }

    // 检查目录是否存在
    struct stat statbuf;
    if (stat(dirPath.c_str(), &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        logWarn << "Directory does not exist or is not a directory: " << dirPath;
        return;
    }

    uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_MODIFY;
    int wd = inotify_add_watch(_inotifyFd, dirPath.c_str(), mask);

    if (wd == -1) {
        logError << "Failed to add watch for directory " << dirPath << ": " << strerror(errno);
        return;
    }

    _watchDescriptors[wd] = dirPath;
    _directoryWatches[dirPath] = wd;

    logDebug << "Added watch for directory: " << dirPath << " (wd: " << wd << ")";
}

void AutoVideoStreamer::removeWatchDirectory(const string& dirPath)
{
    auto it = _directoryWatches.find(dirPath);
    if (it == _directoryWatches.end()) {
        logDebug << "Directory not being watched: " << dirPath;
        return;
    }

    int wd = it->second;
    if (inotify_rm_watch(_inotifyFd, wd) == -1) {
        logWarn << "Failed to remove watch for directory " << dirPath << ": " << strerror(errno);
    }

    _watchDescriptors.erase(wd);
    _directoryWatches.erase(it);

    logDebug << "Removed watch for directory: " << dirPath << " (wd: " << wd << ")";
}

void AutoVideoStreamer::fileMonitorThread()
{
    logInfo << "File monitor thread started";

    const size_t bufferSize = 4096;
    char buffer[bufferSize];

    while (_monitorRunning.load()) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_inotifyFd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;  // 1秒超时
        timeout.tv_usec = 0;

        int ret = select(_inotifyFd + 1, &readfds, nullptr, nullptr, &timeout);

        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            logError << "select() failed in file monitor: " << strerror(errno);
            break;
        }

        if (ret == 0) {
            // 超时，检查稳定性
            processStabilityCheck();
            continue;
        }

        if (FD_ISSET(_inotifyFd, &readfds)) {
            ssize_t length = read(_inotifyFd, buffer, bufferSize);
            if (length == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }
                logError << "read() failed in file monitor: " << strerror(errno);
                break;
            }

            // 解析inotify事件
            for (ssize_t i = 0; i < length; ) {
                struct inotify_event* event = (struct inotify_event*)&buffer[i];

                if (event->len > 0) {
                    string fileName = event->name;
                    auto dirIt = _watchDescriptors.find(event->wd);

                    if (dirIt != _watchDescriptors.end()) {
                        string filePath = dirIt->second + "/" + fileName;

                        // 只处理.mp4文件
                        if (isSupportedVideoFile(filePath)) {
                            FileEvent fileEvent;
                            fileEvent.filePath = filePath;
                            fileEvent.fileName = extractFileName(filePath);
                            fileEvent.cookie = event->cookie;
                            fileEvent.timestamp = chrono::duration_cast<chrono::milliseconds>(
                                chrono::steady_clock::now().time_since_epoch()).count();

                            if (event->mask & IN_CREATE) {
                                fileEvent.type = FileEventType::FILE_CREATED;
                            } else if (event->mask & IN_DELETE) {
                                fileEvent.type = FileEventType::FILE_DELETED;
                            } else if (event->mask & IN_MOVED_FROM) {
                                fileEvent.type = FileEventType::FILE_MOVED_FROM;
                            } else if (event->mask & IN_MOVED_TO) {
                                fileEvent.type = FileEventType::FILE_MOVED_TO;
                            } else if (event->mask & IN_MODIFY) {
                                fileEvent.type = FileEventType::FILE_MODIFIED;
                            } else {
                                i += sizeof(struct inotify_event) + event->len;
                                continue;
                            }

                            // 将事件加入队列
                            {
                                lock_guard<mutex> lck(_eventMtx);
                                _eventQueue.push(fileEvent);
                                _eventCv.notify_one();
                            }

                            logDebug << "File event: " << static_cast<int>(fileEvent.type)
                                    << " for file: " << filePath;
                        }
                    }
                }

                i += sizeof(struct inotify_event) + event->len;
            }
        }

        // 处理事件队列
        processFileEvents();
    }

    logInfo << "File monitor thread stopped";
}

void AutoVideoStreamer::processFileEvents()
{
    unique_lock<mutex> lck(_eventMtx);

    while (!_eventQueue.empty()) {
        FileEvent event = _eventQueue.front();
        _eventQueue.pop();
        lck.unlock();

        handleFileEvent(event);

        lck.lock();
    }
}

void AutoVideoStreamer::handleFileEvent(const FileEvent& event)
{
    // 检查并发操作限制
    if (_currentFileOps.load() >= _maxConcurrentFileOps) {
        logWarn << "Too many concurrent file operations, skipping event for: " << event.filePath;
        return;
    }

    switch (event.type) {
        case FileEventType::FILE_CREATED:
        case FileEventType::FILE_MODIFIED:
            {
                // 对于创建和修改事件，添加到待处理列表进行防抖
                lock_guard<mutex> lck(_pendingMtx);
                auto& pendingInfo = _pendingFiles[event.filePath];
                pendingInfo.filePath = event.filePath;
                pendingInfo.fileName = event.fileName;
                pendingInfo.lastEventTime = event.timestamp;
                pendingInfo.isStable = false;

                logDebug << "Added file to pending list: " << event.filePath;
            }
            break;

        case FileEventType::FILE_DELETED:
            {
                // 立即处理删除事件
                onFileDeleted(event.filePath, event.fileName);

                // 从待处理列表中移除
                lock_guard<mutex> lck(_pendingMtx);
                _pendingFiles.erase(event.filePath);
            }
            break;

        case FileEventType::FILE_MOVED_FROM:
            {
                // 记录移动源文件
                lock_guard<mutex> lck(_pendingMtx);
                _moveFromFiles[event.cookie] = event.filePath;

                // 从待处理列表中移除
                _pendingFiles.erase(event.filePath);

                onFileMovedFrom(event.filePath, event.fileName, event.cookie);
            }
            break;

        case FileEventType::FILE_MOVED_TO:
            {
                lock_guard<mutex> lck(_pendingMtx);
                auto moveIt = _moveFromFiles.find(event.cookie);

                if (moveIt != _moveFromFiles.end()) {
                    // 这是一个重命名操作
                    string oldFilePath = moveIt->second;
                    _moveFromFiles.erase(moveIt);

                    onFileMovedTo(event.filePath, event.fileName, event.cookie);
                } else {
                    // 这是一个从外部移入的文件，当作创建处理
                    auto& pendingInfo = _pendingFiles[event.filePath];
                    pendingInfo.filePath = event.filePath;
                    pendingInfo.fileName = event.fileName;
                    pendingInfo.lastEventTime = event.timestamp;
                    pendingInfo.isStable = false;
                }
            }
            break;
    }
}

void AutoVideoStreamer::processStabilityCheck()
{
    uint64_t currentTime = chrono::duration_cast<chrono::milliseconds>(
        chrono::steady_clock::now().time_since_epoch()).count();

    vector<string> stableFiles;

    {
        lock_guard<mutex> lck(_pendingMtx);

        for (auto& pair : _pendingFiles) {
            auto& pendingInfo = pair.second;

            if (!pendingInfo.isStable &&
                (currentTime - pendingInfo.lastEventTime) >= _fileStabilityDelayMs) {

                // 检查文件是否真的存在且可读
                struct stat statbuf;
                if (stat(pendingInfo.filePath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
                    pendingInfo.isStable = true;
                    stableFiles.push_back(pendingInfo.filePath);

                    logDebug << "File is now stable: " << pendingInfo.filePath;
                }
            }
        }
    }

    // 处理稳定的文件
    for (const auto& filePath : stableFiles) {
        string fileName = extractFileName(filePath);
        onFileCreated(filePath, fileName);

        // 从待处理列表中移除
        lock_guard<mutex> lck(_pendingMtx);
        _pendingFiles.erase(filePath);
    }
}

void AutoVideoStreamer::onFileCreated(const string& filePath, const string& fileName)
{
    _currentFileOps.fetch_add(1);

    logInfo << "Processing file creation: " << filePath;

    // 检查文件是否已经存在推流
    {
        lock_guard<mutex> lck(_mtx);
        if (_streamInfoMap.find(fileName) != _streamInfoMap.end()) {
            logWarn << "Stream already exists for file: " << fileName;
            _currentFileOps.fetch_sub(1);
            return;
        }
    }

    // 创建媒体源
    if (createMediaSourceForVideo(filePath, fileName)) {
        logInfo << "Successfully created stream for new file: " << fileName
                << " -> " << generateRtspUrl(fileName);
    } else {
        logError << "Failed to create stream for new file: " << filePath;
    }

    _currentFileOps.fetch_sub(1);
}

void AutoVideoStreamer::onFileDeleted(const string& filePath, const string& fileName)
{
    _currentFileOps.fetch_add(1);

    logInfo << "Processing file deletion: " << filePath;

    // 移除对应的推流
    {
        lock_guard<mutex> lck(_mtx);
        auto it = _streamInfoMap.find(fileName);
        if (it != _streamInfoMap.end()) {
            logInfo << "Removing stream for deleted file: " << fileName
                    << " (" << it->second.rtspUrl << ")";
            _streamInfoMap.erase(it);
        } else {
            logDebug << "No stream found for deleted file: " << fileName;
        }
    }

    _currentFileOps.fetch_sub(1);
}

void AutoVideoStreamer::onFileMovedFrom(const string& filePath, const string& fileName, uint32_t cookie)
{
    logInfo << "Processing file move from: " << filePath << " (cookie: " << cookie << ")";

    // 暂时移除推流，等待MOVED_TO事件
    {
        lock_guard<mutex> lck(_mtx);
        auto it = _streamInfoMap.find(fileName);
        if (it != _streamInfoMap.end()) {
            logDebug << "Temporarily removing stream for moved file: " << fileName;
            _streamInfoMap.erase(it);
        }
    }
}

void AutoVideoStreamer::onFileMovedTo(const string& filePath, const string& fileName, uint32_t cookie)
{
    _currentFileOps.fetch_add(1);

    logInfo << "Processing file move to: " << filePath << " (cookie: " << cookie << ")";

    // 检查文件是否已经存在推流
    {
        lock_guard<mutex> lck(_mtx);
        if (_streamInfoMap.find(fileName) != _streamInfoMap.end()) {
            logWarn << "Stream already exists for moved file: " << fileName;
            _currentFileOps.fetch_sub(1);
            return;
        }
    }

    // 创建新的媒体源
    if (createMediaSourceForVideo(filePath, fileName)) {
        logInfo << "Successfully created stream for moved file: " << fileName
                << " -> " << generateRtspUrl(fileName);
    } else {
        logError << "Failed to create stream for moved file: " << filePath;
    }

    _currentFileOps.fetch_sub(1);
}
