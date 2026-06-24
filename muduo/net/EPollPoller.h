// EPollPoller.h
#pragma once

#include "Poller.h"
#include "Timestamp.h"
#include <vector>
#include <sys/epoll.h>

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    // 重写基类的方法
    Timestamp poll(int timeoutMs, ChannelList& activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    // 默认的 epoll_event 个数
    static const int kInitEventListSize = 16;

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList& activeChannels) const;
    // 更新 Channel 
    void update(int operation, Channel* channel);

    int epollfd_;
    std::vector<struct epoll_event> events_;    
};