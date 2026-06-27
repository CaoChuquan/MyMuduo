#pragma once
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"
#include <functional>

class EventLoop;
class InetAddress;

/* Acceptor 类负责监听一个本地端口，并在有新的客户端连接请求到达时，
接受连接并将得到的已连接 socket 文件描述符转交给上层业务处理。它是 
TcpServer 实现服务端监听功能的核心组件。 */
class Acceptor : noncopyable {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAdd, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = std::move(cb);
    }

    bool listenning() const { return listenning_; }
    void listen();
private:
    void handleRead();

    EventLoop* loop_; // 使用用户定义的 baseLoop,即 mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_{ false };
};