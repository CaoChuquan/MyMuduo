#pragma once

#include "noncopyable.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

class EventLoop;
class EventLoopThread;

/* 主要作用：
1. 管理 I/O 线程池：管理 EventLoopThread 集合，每个 EventLoopThread 对象都封装了一个
独立的线程和该线程专属的 EventLoop 事件循环，当调用 start() 后，线程池会创建并启动它们

2. 分配新连接：当 TcpServer 收到一个新连接时，EventLoopThreadPool 负责决定将这个连接
交给那个工作线程处理

3. 实现负载均衡：默认采用轮询算法分配新连接，使得每个线程分配的 EventLoop 数量大致相同，
实现负载均衡

4. 灵活讲解：当设置的线程数为 0 时，线程池降级为单线程模式，所工作都在 baseLoop（主线程)
中完成
*/

class EventLoopThreadPool : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool() { /* */ }

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    
    EventLoop* getNextLoop();
    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }
    const std::string name() const { return name_; }
private:
    EventLoop* baseLoop_;  // 主线程
    std::string name_;
    bool started_{ false };
    int numThreads_{ 0 };
    int next_{ 0 };
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;  // 工作线程
};