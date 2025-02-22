#include "lemon/net/inetaddress.h"

using namespace lemon;
using namespace lemon::net;

InetAddress::InetAddress(std::string ip, uint16_t port) {
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(sockaddr_in addr): m_addr(addr) {}

InetAddress::~InetAddress() {}

uint16_t InetAddress::getPort() const {
    return ntohs(m_addr.sin_port);
}

std::string InetAddress::getIp() const {
    return inet_ntoa(m_addr.sin_addr);
}

std::string InetAddress::getIpPort() const {
    return getIp() + ":" + std::to_string(getPort());
}

const sockaddr* InetAddress::getAddr() const  {
    return (sockaddr*)&m_addr;
}