#include "lemon/net/tcp_server.h"

#include <functional>
#include <iostream>

using namespace lemon;
using namespace lemon::net;

class EchoServer {
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
        : m_tcpServer(loop, listenAddr, name) {
        m_tcpServer.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        m_tcpServer.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        m_tcpServer.setThreadNum(3);
    }

    void start() {
        
        m_tcpServer.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            std::cout << conn->peerAddress().getIpPort() << " -> "
                      << conn->localAddress().getIpPort() << " is on" << std::endl;
        } else {
            std::cout << conn->peerAddress().getIpPort() << " -> "
                      << conn->localAddress().getIpPort()<< " is off" << std::endl;
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
        std::string msg = buf->retrieveAllAsString();
        std::cout << "time " << receiveTime.toString() << " , received msg: " << msg << std::endl;
        conn->send(msg);
    }

    TcpServer m_tcpServer;
};


int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 1234);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();
    return 0;
}