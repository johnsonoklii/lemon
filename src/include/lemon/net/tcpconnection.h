#ifndef LEMON_TCPCONNECTION_H
#define LEMON_TCPCONNECTION_H

#include "lemon/net/channel.h"
#include "lemon/net/inetaddress.h"
#include "lemon/net/callbacks.h"
#include "lemon/net/buffer.h"

#include "lemon/base/nocopyable.h"

#include <string>
#include <memory>

namespace lemon {
namespace net {

class EventLoop;
class Socket;

class TcpConnection: noncopyable
                    , public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop
                , const std::string& name
                , int sockfd
                , int triMode
                , const InetAddress& localAddr
                , const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return m_loop; }
    const std::string& name() const { return m_name; }
    
    const InetAddress& localAddress() const { return m_localAddr; }
    const InetAddress& peerAddress() const { return m_peerAddr; }

    bool connected() const { return m_state == kConnected; }
    bool disconnected() const { return m_state == kDisconnected; }
    
    void send(const std::string& message);
    void shutdown();

    void startRead();
    void stopRead();

    bool isReading() const { return m_reading; }

    void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }
    void setCloseCallback(const CloseCallback& cb) { m_closeCallback = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        m_highWaterMarkCallback = cb; m_highWaterMark = highWaterMark; 
    }

    Buffer* inputBuffer() { return &m_inputBuffer; }
    Buffer* outputBuffer() { return &m_outputBuffer; }
    
    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    
    // FIXME: delete
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    
    void handleClose();
    void handleError();

    void handleReadET(Timestamp receiveTime);
    void handleReadLT(Timestamp receiveTime);
    void handleWriteET();
    void handleWriteLT();

    void sendInLoop(const std::string& message);

    void shutdownInLoop();
    void setState(StateE s) { m_state = s; }
    void startReadInLoop();
    void stopReadInLoop();

    EventLoop* m_loop;
    const std::string m_name;
    StateE m_state;
    bool m_reading;
    int m_triMode;     // 0: LT, 1: ET

    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;
    
    const InetAddress m_localAddr;
    const InetAddress m_peerAddr;
    
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    CloseCallback m_closeCallback;

    size_t m_highWaterMark;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
};

} // namespace net
} // namespace lemon


#endif