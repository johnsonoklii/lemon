#include "lemon/net/socket.h"
#include "lemon/net/inetaddress.h"
#include "lemon/base/logger/logger.h"

#include <cstring>

#include <netinet/tcp.h>
#include <fcntl.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

namespace lemon {
namespace net {
    sockaddr_in getLocalAddr(int sockfd) {
        struct sockaddr_in localaddr;
        bzero(&localaddr, sizeof(localaddr));
        socklen_t addrlen = static_cast<socklen_t>(sizeof(sockaddr_in));
        if (::getsockname(sockfd, (sockaddr*)&localaddr, &addrlen)) {
            LOG_ERROR("sockets::getLocalAddr\n");
        }
        return localaddr;
    }
}
}



int Socket::createSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
      LOG_FATAL("sockets::createNonblockingOrDie\n");
    }
    return sockfd;
}

Socket::~Socket() {
    if (::close(m_fd) < 0) {
        LOG_ERROR("Socket::~Socket()\n");
    }
}

void Socket::bind(const InetAddress& addr) {
    int ret = ::bind(m_fd, addr.getAddr(), sizeof(sockaddr));
    if (ret < 0) {
        LOG_FATAL("Socket::bind()\n");
    }
}

void Socket::listen() {
    int ret = ::listen(m_fd, SOMAXCONN);
    if (ret < 0) {
        LOG_FATAL("Socket::listen()\n");
    }
}
int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr; 
    bzero(&addr, sizeof(addr));
    socklen_t addrlen = sizeof(sockaddr);
    int cfd = ::accept4(m_fd, (sockaddr*)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (cfd < 0) {
        int save_errno = errno;
        LOG_ERROR("Socket::accept()\n");
        switch (save_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = save_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL("unexpected error of ::accept %d\n", save_errno);
                break;
            default:
                LOG_FATAL("unknown error of ::accept %d\n", save_errno);
                break;
        }
    }

    if (cfd >= 0) {
        peeraddr->setAddr(addr);
    }

    return cfd;
}

void Socket::shutdownWrite() {
    if (::shutdown(m_fd, SHUT_WR) < 0) {
        LOG_ERROR("Socket::shutdownWrite()\n");
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY,
                &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR,
                &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT,
                            &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_ERROR("SO_REUSEPORT failed\n");
    }
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE,
                &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setNonBlock() {
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags == -1) {
        LOG_ERROR("Socket::setNonBlock(): Failed to get flags for fd\n");
        return;
    }
    if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR("Socket::setNonBlock(): Failed to set non-blocking mode for fd\n");
    }
}