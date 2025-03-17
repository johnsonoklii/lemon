#ifndef LEMOM_SSL_CONNECTION_H
#define LEMOM_SSL_CONNECTION_H
#include "lemon/net/http/ssl/ssl_context.h"
#include "lemon/net/tcpconnection.h"

#include <memory>

namespace lemon {
namespace http {
namespace ssl {

class SSLConnection {
public:
    using TcpConnectionPtr = std::shared_ptr<lemon::net::TcpConnection>;
    using BufferPtr = lemon::net::Buffer*;

    using MessageCallback = std::function<void(const std::shared_ptr<lemon::net::TcpConnection>&,
                                                lemon::net::Buffer*,
                                                lemon::base::Timestamp)>;
public:
    SSLConnection(const TcpConnectionPtr& conn, SSLContext* ctx);
    ~SSLConnection();

    void startHandshake();
    void send(const void* data, size_t len);
    void onRead(const TcpConnectionPtr& conn, BufferPtr buf, lemon::base::Timestamp time);

    bool isHandshakeCompleted() const { return m_state == SSLState::ESTABLISHED; }

    BufferPtr getDecryptedBuffer() { return &m_decryptedBuffer; }

    // SSL BIO 操作回调
    static int bioWrite(BIO* bio, const char* data, int len);
    static int bioRead(BIO* bio, char* data, int len);
    static long bioCtrl(BIO* bio, int cmd, long num, void* ptr);

    // 设置消息回调函数
    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = std::move(cb); }

private:
    void handleHandshake();
    void onEncrypted(const char* data, size_t len);
    void onDecrypted(const char* data, size_t len);
    SSLError getLastError(int ret);
    void handleError(SSLError error);
private:
    SSL*                m_ssl;              // SSL 连接
    SSLContext*         m_ctx;              // SSL 上下文
    TcpConnectionPtr    m_conn;             // TCP 连接
    SSLState            m_state;            // SSL 状态
    BIO*                m_readBio;          // 网络数据 -> SSL
    BIO*                m_writeBio;         // SSL -> 网络数据
    lemon::net::Buffer  m_readBuffer;       // 读缓冲区
    lemon::net::Buffer  m_writeBuffer;      // 写缓冲区
    lemon::net::Buffer  m_decryptedBuffer;  // 解密后的数据
    MessageCallback     m_messageCallback;  // 消息回调
};

}
}
}

#endif