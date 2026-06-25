#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include <cstdlib>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg) 
    : name_(nameArg), baseLoop_(baseLoop) {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    // 单线程模式
    if (numThreads_ ==  0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;

        // 轮询到最后一个 EventLoop 对象后从头开始
        if (next_ >= loops_.size()) next_ = 0;
    }
    
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, baseLoop_); // 只有一个线程
    }
    else {
        return loops_;
    }
}