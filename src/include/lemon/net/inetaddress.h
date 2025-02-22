#ifndef LEMON_INETADDRESS_H
#define LEMON_INETADDRESS_H

#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>

namespace lemon {
namespace net {

class InetAddress {
public:
    InetAddress() = default;
    InetAddress(std::string ip, uint16_t port);
    InetAddress(sockaddr_in addr);
    ~InetAddress();

    uint16_t getPort() const;
    std::string getIp() const;
    std::string getIpPort() const;
    const sockaddr* getAddr() const;
    void setAddr(const sockaddr_in& addr) { m_addr = addr; }
    sa_family_t family() const { return m_addr.sin_family; }

private:
    sockaddr_in m_addr;
};

} // namespace net
} // namespace lemon

#endif