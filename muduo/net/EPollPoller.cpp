//EPollPoller.cpp
#include "Channel.h"
#include "EPollPoller.h"
#include "Logger.h"
#include <cstring>
#include <unistd.h>
#include <errno.h>

/* 用于准确记录 Channel 的生命周期，以确保对底层的
   epoll 实例执行正确的添加、修改和删除操作 */
const int kNew = -1;    // Channel 未添加到 epoll 实例中
const int kAdded = 1;   // Channel 已添加到 epoll 实例中
const int kDeleted = 2; // Channel 从 epoll 中删除，但仍记录在 channels_ 中

EPollPoller::EPollPoller(EventLoop* loop) 
    : Poller(loop), epollfd_(-1), events_(kInitEventListSize) 
{
    epollfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollfd_ == -1) {
        LOG_FATAL("epoll create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller() {
    close(epollfd_);
}

void EPollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    const int fd = channel->ret_eventFd();
    LOG_INFO("fd = %d, events = %d, index = %d\n",
        fd, channel->ret_events(), index);

    if (index == kNew || index == kDeleted) {
        if (index == kNew) {
            // 加入 Poller 的 ChannelList
            channels_[fd] = channel;
        }
        // 加入 epoll
        update(EPOLL_CTL_ADD, channel);
        channel->set_index(kAdded);
    } else {
        if (channel->isNoneEvent()) {
            // 没有感兴趣的事件，从 epoll 中移除
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::update(int op, Channel* channel) {
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n",
        __FUNCTION__, channel->ret_eventFd(), channel->ret_events(), channel->index());

    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.data.ptr = channel;
    e.events = channel->ret_events();

    if (epoll_ctl(epollfd_, op, channel->ret_eventFd(), &e) == -1) {
        if (op == EPOLL_CTL_DEL) 
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        else
            LOG_FATAL("epoll_ctl mod or add error:%d", errno);
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, channel->ret_eventFd());

    if (channel->index() == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channels_.erase(channel->ret_eventFd());
    channel->set_index(kNew);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList& activeChannels) {
    int size = static_cast<int>(events_.size());
    int numEvents = epoll_wait(epollfd_, events_.data(), size, timeoutMs); 
    // 防御型编程：epoll_wait() 失败后，错误码不能丢失。
    // 后续日志输出、条件判断都可能改变 errno，所以必须先保存
    int saveErrno = errno;

    Timestamp now(Timestamp::now());

    if (numEvents > 0) {
        LOG_INFO("%d events happened \n", numEvents);

        // 将有事件的 Channel 加入 activeChannels
        // fillActiveChannels(numEvents, activeChannels);
        Channel* channel = nullptr;
        for (int i = 0; i < numEvents; ++i) {
            channel = static_cast<Channel*>(events_[i].data.ptr);
            // 设置具体发生了什么事件（感兴趣的事件 -> 发生的事件）
            channel->set_events(events_[i].events);
            activeChannels.push_back(channel);
        }

        // 如果就绪的事件（numEvents）与事件总数相等，
        // 很可能说明还有就绪事件在内核中等待，所以需要扩容，
        // 下一次 epoll_wait() 可以将所有就绪事件取出
        if (numEvents == size) {
            events_.resize(size * 2);
        }
    }
    else if (numEvents == 0) {
        LOG_INFO("%s, timeout\n", __FUNCTION__);
    }
    else {
        // 忽略 EINTR 错误码
        if (saveErrno =! EINTR) {
            errno == saveErrno;
            LOG_ERROR("%s epoll_wait err\n", __FUNCTION__);
        }
    }

    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList& activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        // 设置具体发生了什么事件（感兴趣的事件 -> 发生的事件）
        channel->set_events(events_[i].events);
        activeChannels.push_back(channel);
    }
}