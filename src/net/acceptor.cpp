#include "lemon/net/acceptor.h"
#include "lemon/net/eventloop.h"
#include "lemon/net/inetaddress.h"
#include "lemon/net/socket.h"

#include "lemon/base/logger/logger.h"

#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
: m_loop(loop)
 , m_listen_socket(Socket::createSocket(listenAddr.family()))
 , m_channel(loop, m_listen_socket.fd())
 , m_listening(false)
 , m_idle_fd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(m_idle_fd >= 0);

    m_listen_socket.setReuseAddr(true);
    m_listen_socket.setReusePort(reuseport);
    m_listen_socket.bind(listenAddr);

    m_channel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    m_channel.disableAll();
    m_channel.remove();
     ::close(m_idle_fd);
}

void Acceptor::listen() {
    m_loop->assertInLoopThread();
    m_listening = true;
    m_listen_socket.listen();
    m_channel.enableReading();
}

void Acceptor::handleRead() {
    m_loop->assertInLoopThread();
    InetAddress peeraddr;
    int connfd = m_listen_socket.accept(&peeraddr);

    if (connfd >= 0) {
        if (m_newConnectionCallback) {
            m_newConnectionCallback(connfd, peeraddr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR("Acceptor::handleRead\n");
        if (errno == EMFILE) {
            ::close(m_idle_fd);
            m_idle_fd = ::accept(m_listen_socket.fd(), nullptr, nullptr);
            ::close(m_idle_fd);
            m_idle_fd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}