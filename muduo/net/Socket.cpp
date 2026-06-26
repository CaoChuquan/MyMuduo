#include "Socket.h"
#include <InetAddress.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

Socket::~Socket() {
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr) {
    if (::bind(sockfd_, (sockaddr*)&localaddr, sizeof(sockaddr_in)) < 0) {
        LOG_FATAL("bind sockfd:%d failed\n", sockfd_);
    }
}

void Socket::listen() {
    if (::listen(sockfd_, 1024) < 0) {
        LOG_FATAL("listen sockfd:%d failed\n", sockfd_);
    }
}

int Socket::accept(InetAddress* peeraddr) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int connfd = ::accept(sockfd_, (sockaddr*)&addr, &len);
    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }

    return connfd;
}

void Socket::shutdownWrite() {
    if (shutdown(sockfd_, 1) < 0) {
        LOG_ERROR("shutdown eror\n");
    }
}

// 启用或禁用 Nagle 算法
// 禁用：允许小数据包立即发送（不等待合并），适用于对延迟敏感的应用
//（如交互式游戏、实时通信）。
// 启用：会合并多个小包以减少网络拥塞，但可能增加延迟。
void Socket::setTcpNoDelay(bool on) {
    int optval = int(on);
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

// 是否重用本地地址和端口。
// 当 on = true 时，即使 socket 处于 TIME_WAIT 状态，也可以立即
// 绑定同一地址和端口（常用于服务器快速重启）。
void Socket::setReuseAddr(bool on) {
    int optval = int(on);
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

// SO_REUSEPORT 允许多个 socket 绑定到完全相同的地址和端口，
// 实现端口共享，常用于多进程/多线程服务器中实现连接的负载均
// 衡（内核将传入连接分发到不同的监听 socket）。
void Socket::setReusePort(bool on) {
    int optval = int(on);
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

// 当 on = true 时，如果连接长时间空闲，内核会定期发送探测包
// 以检测对端是否存活（若对端无响应，则关闭连接），常用于清理死连接。
void Socket::setKeepAlive(bool on) {
    int optval = int(on);
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}