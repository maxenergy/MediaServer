#include <cstdlib>
#include <string>
#include <algorithm>
#include <cctype>

#include "Common/Config.h"
#include "RecordReaderBase.h"
#include "Logger.h"
#include "Util/String.h"
#include "WorkPoller/WorkLoopPool.h"

using namespace std;

function<RecordReaderBase::Ptr(const string& path)> RecordReaderBase::_createRecordReader;

void RecordReaderBase::registerCreateFunc(const function<RecordReaderBase::Ptr(const string& path)>& func)
{
    _createRecordReader = func;
}

RecordReaderBase::Ptr RecordReaderBase::createRecordReader(const string& path)
{
    logDebug << "RecordReaderBase::createRecordReader called with path: " << path;

    if (!_createRecordReader) {
        logError << "RecordReaderBase::createRecordReader - _createRecordReader function is null! RecordReader::init() may not have been called.";
        return nullptr;
    }

    logDebug << "RecordReaderBase::createRecordReader - calling registered creation function";
    auto result = _createRecordReader(path);

    if (result) {
        logDebug << "RecordReaderBase::createRecordReader - successfully created reader for path: " << path;
    } else {
        logError << "RecordReaderBase::createRecordReader - creation function returned nullptr for path: " << path;
    }

    return result;
}