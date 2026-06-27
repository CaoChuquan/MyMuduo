#include "Acceptor.h"
#include "Logger.h"
#include "Timestamp.h"
#include "InetAddress.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

static int createNoblocking() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_FATAL("socket error\n");
    }

    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAdd, 
    bool reuseport) 
    : loop_(loop)
    , acceptSocket_(createNoblocking())
    , acceptChannel_(loop, acceptSocket_.fd())
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAdd);

    acceptChannel_.setReadCallback([this](Timestamp) { this->handleRead(); });
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    // 将 Channel 从 EventLoop 的 Poller 中彻底移除
    acceptChannel_.remove();
}

void Acceptor::listen() {
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

// listenfd 发生事件，即有新用户连接
void Acceptor::handleRead() {
    InetAddress addr;

    int connfd = acceptSocket_.accept(&addr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, addr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR("%s:%s:%d accept error%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE) { // 如果文件描述符达到上限
            LOG_ERROR("socked reached limit\n");
        }
    }
}