// EventLoop.h
#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Poller;
class Channel;

/* EventLoop 类驱动了整个事件循环，负责监听 I/O 事件、执行回调任务
和管理定时器，是连接 Poller、Channel 和上层业务逻辑的总调度者。

核心作用：
1. 事件监听与分发：通过内部的 Poller 等待 I/O 事件发生，并将就绪的 
Channel 分发出去处理。
2. 任务执行：执行用户通过 runInLoop() 或 queueInLoop() 投递到该线
程的任务（Functor）。
3. 定时器管理：管理定时任务，利用 timerfd 机制在指定时间触发回调。

EventLoop 与线程绑定，一个线程只能有一个 EventLoop 实例
（通过 thread_local 保证），所有操作必须在该线程内完成，这极大简化了
并发模型。 */

class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop(); // 开启事件循环
    void quit(); // 退出事件循环
    
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(Functor cb);   // 在当前 loop 中执行回调
    void queueInLoop(Functor cb); // 把 cb 加入队列，唤醒 loop 所在线程执行 cb
    
    void wakeup(); // 唤醒 loop 所在线程

    // 调用 Poller 的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    
    // 判断 EventLoop 对象是否在自己的线程中
    bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid();
    }
private:
    void handleRead();
    void doPendingFunctors();

    std::atomic_bool looping_{ false };
    std::atomic_bool quit_{ false };

    Timestamp pollReturnTime_; // poller 返回发生事件的 channel 的事件节点
    
    const pid_t threadId_{ CurrentThread::tid() };  // 记录当前 loop 所在线程 id

    int wakeupFd_; // 当 mainLoop 获取一个新用户的 channel，通过轮询算法选择一个 subloop
    
    std::unique_ptr<Poller> poller_; // 一个 EventLoop 对应一个 Poller
    std::unique_ptr<Channel> wakeupChannel_;

    // Channel* currentActiveChannels_{ nullptr };
    std::vector<Channel*> activeChannels_;
    std::vector<Functor> pendingFunctors_; // 存储回调操作
    std::mutex mutex_; // 用于保护 vector 容器的线程安全

    std::atomic_bool callingPendingFunctors_{ false }; // 标识 loop 是否有需要回调的操作
};