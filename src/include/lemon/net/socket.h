#ifndef LEMON_SOCKET_H
#define LEMON_SOCKET_H

#include "lemon/net/inetaddress.h"

#include <sys/socket.h>

namespace lemon {
namespace net {

sockaddr_in getLocalAddr(int sockfd);

class Socket {
public:
    explicit Socket(int sockfd): m_fd(sockfd) {}
    ~Socket();

    int fd() const { return m_fd; }

    void bind(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();
    
    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

    static int createSocket(sa_family_t family);

private:
    const int m_fd;
};


} // namespace net
} // namespace lemon

#endif