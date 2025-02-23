#include "lemon/net/tcp_server.h"
#include "lemon/logger/logger.h"

#include <functional>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

TcpServer::TcpServer(EventLoop* loop,
                    const InetAddress& listenAddr,
                    const std::string& name,
                    TRIMODE triMode,
                    Option option)
: m_loop(loop)
 , m_ip_port(listenAddr.getIpPort())
 , m_name(name)
 , m_triMode(triMode)
 , m_acceptor(new Acceptor(loop, listenAddr, option == kReusePort))
 , m_thread_pool(new EventLoopThreadPool(loop, name))
 , m_next_connid(1)  {
    m_acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)
    );
}

TcpServer::~TcpServer() {
    m_loop->assertInLoopThread();
    LOG_INFO("TcpServer::~TcpServer [%s] stoped\n", m_name.c_str());

    for (auto& item : m_connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            // 这里要绑定智能指针，否则在sub loop线程中执行connectDestroyed时，conn就析构了
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int numThreads) {
    m_thread_pool->setThreadNum(numThreads);
}

void TcpServer::start() {
    std::call_once(m_started, [this]{
        m_thread_pool->start(m_threadInitCallback);
        m_loop->runInLoop(
            std::bind(&Acceptor::listen, m_acceptor.get())
        );
    });
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    m_loop->assertInLoopThread();
    EventLoop* sub_loop = m_thread_pool->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", m_ip_port.c_str(), m_next_connid);
    ++m_next_connid;
    std::string conn_name = m_name + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n",
             m_name.c_str(), conn_name.c_str(), peerAddr.getIpPort().c_str());

    InetAddress localAddr(getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(sub_loop, conn_name, sockfd, m_triMode, localAddr, peerAddr));

    m_connections[conn_name] = conn;
    conn->setConnectionCallback(m_connctionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)
    );
    
    sub_loop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    m_loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    m_loop->assertInLoopThread();
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection [%s] from %s\n",
             m_name.c_str(), conn->name().c_str(), conn->peerAddress().getIpPort().c_str());
    
    size_t n = m_connections.erase(conn->name());
    assert(n == 1);
    EventLoop* io_loop = conn->getLoop();
    io_loop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}