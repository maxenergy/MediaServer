﻿#ifndef Heartbeat_H
#define Heartbeat_H

#include <string>
#include <memory>

#include "EventPoller/EventLoop.h"
#include "Common/HookManager.h"

using namespace std;

class Heartbeat : public enable_shared_from_this<Heartbeat>
{
public:
    using Ptr = shared_ptr<Heartbeat>;

    Heartbeat();
    ~Heartbeat();

    void startAsync();
    void start();
    void getSourceInfo(ServerInfo& info);
    void registerServer();

public:

private:
    EventLoop::Ptr _loop;
};


#endif //Heartbeat_H
