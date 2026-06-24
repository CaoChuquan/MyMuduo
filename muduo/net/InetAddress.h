// InetAddress.h
#pragma once

#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
// #include "logger.h"

class InetAddress {
private:
    struct sockaddr_in addr_;
public:
    InetAddress(uint16_t port, const std::string& ip = "127.0.0.1");
    // 返回 IP（点分十进制）
    std::string toIp() const;
    // 返回端口号
    uint16_t toPort() const;
    // 返回 IP 和端口号
    std::string toIpPort() const;
};