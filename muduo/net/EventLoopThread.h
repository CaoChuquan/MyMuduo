#pragma once

#include "noncopyable.h"
#include "Thread.h"
#include "Thread.h"
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
        const std::string& name = "");
    ~EventLoopThread();
    EventLoop* startLoop();    
private:
    void threadFunc();

    EventLoop* loop_{ nullptr };
    bool exiting_{ false };
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};