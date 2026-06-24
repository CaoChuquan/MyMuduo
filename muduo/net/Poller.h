#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include <vector>
#include <unordered_map>

class EventLoop;
class Channel;

// muduo 库中多路复用事件分发器的核心 I/O 复用模块
class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop) : ownerLoop_(loop) {}
    virtual ~Poller() {}

    // 给所有 I/O 复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList& activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    // 检查给定的 channel 是否被当前 Poller 对象正确管理
    bool hasChannel(Channel* channel) const;

    // 返回 I/O 复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    // key: sockfd，value: sockfd 所属的 Channel
    std::unordered_map<int, Channel*> channels_;
private:
    EventLoop* ownerLoop_;
};