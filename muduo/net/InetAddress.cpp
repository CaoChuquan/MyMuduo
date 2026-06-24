// InetAddress.cpp
#include "InetAddress.h"

using std::string;

InetAddress::InetAddress(uint16_t port, const string& ip) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}
// 返回 IP（点分十进制）
string InetAddress::toIp() const {
    char ip[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, ip, INET_ADDRSTRLEN);
    return ip;
}
// 返回端口号
uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
}
// 返回 IP 和端口号
string InetAddress::toIpPort() const {
    return this->toIp() + ":" + std::to_string(this->toPort());
}