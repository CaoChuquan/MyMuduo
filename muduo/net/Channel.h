#pragma once

#include <memory>
#include <functional>
#include <utility>
#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop; 

/* Channel 类的核心职责是封装一个文件描述符其所关心的事件，并在事件
   发生时，调用相应的业务回调函数。它就像一个桥梁，连接了底层的 I/O 
   多路复用（Poller）和上层的业务逻辑（如 TcpConnection）。 */
class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int eventFd) : loop_(loop), eventFd_(eventFd) {}
    ~Channel() {}

    // fd 得到 Poller 通知后，调用相应的回调方法处理事件
    void handlerEvent(Timestamp receiveTime);

    // 回调方法
    void setReadCallback(ReadEventCallback cb) {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb) {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb) {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb) {
        errorCallback_ =std::move(cb);
    }

    // 防止 Channel 被手动 remove 后，Channel 还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int ret_eventFd() const { return eventFd_; }
    int ret_events() const { return events_; }
    void set_events(int revt) { revents_ = revt; }

    // 设置 fd 相应状态
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }

    // fd 的状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int i) { index_ = i; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }

    // 在 Channel 所属的 EventLoop 中，把当前 Channel 删除
    void remove();
private:
    // fd 的状态
    static const int kNoneEvent;  // 没有事件
    static const int kReadEvent;  // 读事件
    static const int kWriteEvent; // 写事件

    EventLoop* loop_; // 事件循环
    const int eventFd_;
    int events_ = 0;    // 注册感兴趣的事件
    int revents_ = 0;     // Poller 返回的具体发生的事
    int index_ = -1;

    std::weak_ptr<void> tie_;
    bool tied_ = false;

    /* 由于 Channel 能够获得 fd 最终发生的事件 revents，
       所以它可以负责调用具体事件的回调操作 */
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    void update();
    void handleEventWithGuard(Timestamp receiveTime);
};