#ifndef LEMON_TCP_SERVER_H
#define LEMON_TCP_SERVER_H

#include "lemon/base/nocopyable.h"
#include "lemon/net/callbacks.h"
#include "lemon/net/eventloop.h"
#include "lemon/net/inetaddress.h"
#include "lemon/net/acceptor.h"
#include "lemon/net/eventloop_threadpool.h"
#include "lemon/net/tcpconnection.h"
#include "lemon/base/timestamp.h"

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <set>

using namespace lemon::base;

namespace lemon {
namespace net {

class EventLoop;
class EventLoopThreadPool;
class Acceptor;
class InetAddress;



class TcpServer: public noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option {
        kNoReusePort,
        kReusePort,
    };

    enum TRIMODE{
        LT = 0,
        ET
    };

    TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& name,
            TRIMODE triMode = LT,
            int64_t conn_timeout = -1,
            Option option = kNoReusePort);
    
    ~TcpServer();

    const std::string& ipPort() const { return m_ip_port; }
    const std::string& name() const { return m_name; }
    EventLoop* getLoop() const { return m_loop; }
    
    void start();

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { m_threadInitCallback = cb; }

    void setConnectionCallback(const ConnectionCallback& cb) { m_connctionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }

    void setConnTimeout(int64_t conn_timeout);
    int64_t connTimeout() const { return m_conn_timeout; }
    
private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    void timeoutConnection();
    void timeoutConnection2();

    EventLoop* m_loop;
    const std::string m_ip_port;
    const std::string m_name;
    TRIMODE m_triMode;
    std::unique_ptr<Acceptor> m_acceptor;
    std::shared_ptr<EventLoopThreadPool> m_thread_pool;
    
    ConnectionCallback m_connctionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    ThreadInitCallback m_threadInitCallback;
    
    std::once_flag m_started;
    int m_next_connid;
    ConnectionMap m_connections; 
    std::set<std::pair<Timestamp, TcpConnectionPtr>> m_activeConns;// FIXME：conn超时，是否再使用一个最小堆/哈希表记录？这样可以减少遍历
    int64_t m_conn_timeout; // 毫秒
};

}
} // namespace lemon

#endif