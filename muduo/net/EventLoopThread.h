#pragma once

#include "noncopyable.h"
#include "Thread.h"
#include "Thread.h"
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>

class EventLoop;

/* EventLoopThread 类用于将 EventLoop 与一个线程绑定并管理其生命周期
的重要工具。它完美地体现了“One Loop Per Thread”的设计思想。它封装了创
建线程、初始化 EventLoop、同步启动状态以及线程退出的全部细节，使得上层
代码可以像使用普通对象一样使用“一个带线程的 EventLoop”。

主要作用：
1. 创建并启动一个独立的线程：该线程专门用于运行一个 EventLoop 事件循环。
2. 确保线程安全地暴露 EventLoop 指针：允许其他线程（如主线程）获得该 
EventLoop 的指针，以便向其投递任务或添加 Channel。
3. 自动管理线程的生命周期：在构造时启动线程，在析构时安全地退出循环并等
待线程结束。 */
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