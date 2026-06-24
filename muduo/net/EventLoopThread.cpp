#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
    const std::string& name)
    : thread_([this]() { this->threadFunc(); }, name)
    , callback_(cb)
{ 

}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    thread_.start();  // 创建一个新线程

    EventLoop* loop = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

// 单独创建的线程执行此函数
void EventLoopThread::threadFunc() {
    EventLoop loop; // 在此线程中创建 EventLoop，体现：one loop per thread

    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();  // 调用 Poller 监听事件
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}