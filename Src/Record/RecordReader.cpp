#include <cstdlib>
#include <string>
#include <algorithm>
#include <cctype>

#include "Common/Config.h"
#include "RecordReader.h"
#include "Logger.h"
#include "Util/String.h"
#include "WorkPoller/WorkLoopPool.h"
#include "EventPoller/EventLoopPool.h"
#include "RecordReaderMp4.h"
#include "RecordReaderPs.h"

using namespace std;

RecordReader::RecordReader(const string& path)
{

}

RecordReader::~RecordReader()
{
    close();
}

void RecordReader::init()
{
    RecordReaderBase::registerCreateFunc([](const string& path) -> RecordReaderBase::Ptr {
        logDebug << "RecordReader creation function called with path: " << path;

        // Extract extension
        size_t dotPos = path.rfind('.');
        if (dotPos == string::npos) {
            logError << "RecordReader: No file extension found in path: " << path;
            return nullptr;
        }

        string ext = path.substr(dotPos + 1);
        logDebug << "RecordReader: Extracted extension before slash removal: " << ext;

        size_t slashPos = ext.find_first_of("/");
        if (slashPos != string::npos) {
            ext = ext.substr(0, slashPos);
        }

        logDebug << "RecordReader: Final extracted extension: " << ext;

#ifdef ENABLE_MPEG
        if (!strcasecmp(ext.data(), "ps") || !strcasecmp(ext.data(), "mpeg")) {
            logDebug << "RecordReader: Creating RecordReaderPs for extension: " << ext;
            return make_shared<RecordReaderPs>(path);
        }
#endif
#ifdef ENABLE_MP4
        if (!strcasecmp(ext.data(), "mp4")) {
            logDebug << "RecordReader: Creating RecordReaderMp4 for extension: " << ext;
            try {
                auto reader = make_shared<RecordReaderMp4>(path);
                logDebug << "RecordReader: Successfully created RecordReaderMp4";
                return reader;
            } catch (const exception& e) {
                logError << "RecordReader: Exception creating RecordReaderMp4: " << e.what();
                return nullptr;
            }
        }
#endif
        logError << "RecordReader: Unsupported file extension: " << ext << " for path: " << path;
        return nullptr;
    });
}

bool RecordReader::start()
{
    weak_ptr<RecordReader> wSelf = shared_from_this();
    _workLoop = WorkLoopPool::instance()->getLoopByCircle();
    _loop = EventLoop::getCurrentLoop();

    // 如果当前线程没有EventLoop，从EventLoopPool获取一个
    if (!_loop) {
        logWarn << "No current EventLoop, getting one from EventLoopPool";
        _loop = EventLoopPool::instance()->getLoopByCircle();
    }

    _clock.start();

    return true;
}

void RecordReader::stop()
{
    close();
}

void RecordReader::close()
{
    if (_onClose) {
        _onClose();
    }
    _onClose = nullptr;
    _onTrackInfo = nullptr;
    _onReady = nullptr;
    _onFrame = nullptr;
}

void RecordReader::setOnTrackInfo(const function<void(const TrackInfo::Ptr& trackInfo)>& cb)
{
    _onTrackInfo = cb;
}

void RecordReader::setOnReady(const function<void()>& cb)
{
    _onReady = cb;
}

void RecordReader::setOnClose(const function<void()>& cb)
{
    _onClose = cb;
}

void RecordReader::setOnFrame(const function<void(const FrameBuffer::Ptr& frame)>& cb)
{
    _onFrame = cb;
}
