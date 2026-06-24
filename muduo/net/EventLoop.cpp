#include "EventLoop.h"
#include "Poller.h"
#include "Logger.h"
#include "Channel.h"

#include <memory>
#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>

// 防止一个线程创建多个 EventLoop
__thread EventLoop* t_loopInThisThread = nullptr;

// 默认的超时时间
constexpr int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd == -1) {
        LOG_FATAL("eventfd error:%d\n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : wakeupFd_(createEventfd())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupChannel_(new Channel(this, wakeupFd_))
{
    if (t_loopInThisThread) {
        LOG_FATAL("Another EventLoop %p exists in this thread %d\n",
            t_loopInThisThread, threadId_);
    }
    else {
        t_loopInThisThread = this;
    }

    // 设置 wakeupfd 的事件类型以及发生事件后的回调操作
    // wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->setReadCallback([this](Timestamp) { this->handleRead(); });
    // 每一个 EventLoop 都将监听 wakeupChannel 的 EPOLLIN 事件
    wakeupChannel_->enableReading();

}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleRead() read %d bytes insted of 8\n", n);
    }
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping\n", this);
    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, activeChannels_);
        for (Channel* channel : activeChannels_) {
            // Poller 监听哪些 Channel 发生了事件后通知给 EventLoop,
            // EventLoop 在通知 Channel 处理相应的事件
            channel->handlerEvent(pollReturnTime_);
        }
        // 执行当前 EventLoop 事件循环需要处理的回调操作
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping.\n", this);
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;

    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();  // 在当前 Loop 中,执行 cb
    }
    else {   // 不在当前线程中，加入队列
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb) {
    // 多线程访问共享资源，使用互斥锁
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }   

    // add comment
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup(); // 唤醒 Loop 所在线程
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    return poller_->hasChannel(channel);
}

// 向 eventfd 写入一个字节，让阻塞在 epoll_wait() 
// 上的 I/O 线程立即返回，从而有机会执行 doPendingFunctors。
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes insted of 8\n", n);
    }
}

void EventLoop::doPendingFunctors() {
    // 使用一个局部的 functors 变量原因：
    // 1. 缩短锁持有时间，避免阻塞其他线程
    // 2. 避免可能发生的回调函数再次申请锁导致死锁
    // 3. 
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    // 执行回调操作
    for (const Functor& f : functors)  {
        f();
    }

    callingPendingFunctors_ = false;
}