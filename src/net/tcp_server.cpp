#include "lemon/net/tcp_server.h"
#include "lemon/base/logger/logger.h"

#include <functional>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

TcpServer::TcpServer(EventLoop* loop,
                    const InetAddress& listenAddr,
                    const std::string& name,
                    TRIMODE triMode,
                    int64_t conn_timeout,
                    Option option)
: m_loop(loop)
 , m_ip_port(listenAddr.getIpPort())
 , m_name(name)
 , m_triMode(triMode)
 , m_acceptor(new Acceptor(loop, listenAddr, option == kReusePort))
 , m_thread_pool(new EventLoopThreadPool(loop, name))
 , m_next_connid(1)
 , m_conn_timeout(conn_timeout)  {
    m_acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)
    );

    if (m_conn_timeout > 0) {
        m_loop->runEvery(m_conn_timeout, std::bind(&TcpServer::timeoutConnection2, this));
    }
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

    LOG_DEBUG("TcpServer::newConnection [%s] - new connection [%s] from %s\n",
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

    if (m_conn_timeout > 0) {
        conn->setTimeout(m_conn_timeout);
    }

    // std::pair<Timestamp*, TcpConnectionPtr> pair = std::make_pair(conn->lastReadTime(), conn);
    m_activeConns.insert(conn);

    assert(m_connections.size() == m_activeConns.size());
    
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
    LOG_DEBUG("TcpServer::removeConnectionInLoop [%s] - connection [%s] from %s\n",
             m_name.c_str(), conn->name().c_str(), conn->peerAddress().getIpPort().c_str());
    
    size_t n = m_connections.erase(conn->name());
    assert(n == 1);
    
    // std::pair<Timestamp*, TcpConnectionPtr> pair = std::make_pair(conn->lastReadTime(), conn);
    m_activeConns.erase(conn);

    assert(m_connections.size() == m_activeConns.size());

    EventLoop* io_loop = conn->getLoop();
    io_loop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}

void TcpServer::setConnTimeout(int64_t conn_timeout) {
    m_conn_timeout = conn_timeout;
    if (m_conn_timeout > 0) {
        m_loop->runEvery(m_conn_timeout, std::bind(&TcpServer::timeoutConnection2, this));
    }
}

void TcpServer::timeoutConnection() {
    for (auto it : m_connections) {
        Timestamp* last_read_time =  it.second->lastReadTime();

        int64_t pad_sec = Timestamp::now().milliSecondsSinceEpoch() - last_read_time->milliSecondsSinceEpoch();
        int64_t timeout = it.second->timeout();

        LOG_INFO("pad_sec = %ld, timeout = %ld\n", pad_sec, timeout);
        if (timeout < 0) continue;

        if (pad_sec > timeout) {
            LOG_WARN("TcpServer::timeoutConnection [%s] - connection [%s] from %s\n",
                     m_name.c_str(), it.second->name().c_str(), it.second->peerAddress().getIpPort().c_str());
            it.second->forceClose();
       }
    }
}

void TcpServer::timeoutConnection2() {
    Timestamp now = Timestamp::now();
    while (!m_activeConns.empty()) {
        auto it = m_activeConns.begin();
        // int64_t pad = now.milliSecondsSinceEpoch() - it->second->lastReadTime()->milliSecondsSinceEpoch();
        int64_t pad = now.milliSecondsSinceEpoch() - (*it)->lastReadTime()->milliSecondsSinceEpoch();

        if (pad < (*it)->timeout()) break;
        
        LOG_WARN("TcpServer::timeoutConnection2 [%s] - connection [%s] from %s\n",
                 m_name.c_str(), (*it)->name().c_str(), (*it)->peerAddress().getIpPort().c_str());
        (*it)->forceClose(); 
        m_activeConns.erase(it); // FIXME: 这里不移除m_activeConns，会死循环，因为forceClose需要等到下一次才被激活
    }
}