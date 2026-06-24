#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include <sys/epoll.h>

// using namespace std;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

/* 当改变 Channel 所表示 fd 的事件后，update() 
   负责在 Pollar 里面改变 fd 相应的事件 */
void Channel::update() {
/* 通过 Channel 所属的 EventLoop，调用 poller
   的相应方法，注册 fd 的 events 事件 */
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::handlerEvent(Timestamp receiveTime) {
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) handleEventWithGuard(receiveTime);
    }
    else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_INFO("Channel handleEvent revents:%d\n", revents_);

    // 对端关闭并且没有可读事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) closeCallback_();
    }

    // 发生错误
    if (EPOLLERR & revents_) {
        if (errorCallback_) errorCallback_();
    }

    // 可读事件
    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    
    // 可写事件
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    }
}