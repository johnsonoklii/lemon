#include "lemon/net/http/ssl/ssl_connection.h"

#include "lemon/base/logger/logger.h"

#include <openssl/err.h>

using namespace lemon::http::ssl;
using namespace lemon::log;

SSLConnection::SSLConnection(const TcpConnectionPtr& conn, SSLContext* ctx)
: m_ssl(nullptr)
 , m_ctx(ctx)
 , m_conn(conn)
 , m_state(SSLState::HANDSHAKE)
 , m_readBio(nullptr)
 , m_writeBio(nullptr)
 , m_messageCallback(nullptr) {
        // 创建 SSL 对象
    m_ssl = SSL_new(m_ctx->getNativeHandle());
    if (!m_ssl) {
        LOG_INFO("Failed to create SSL object: %s", ERR_error_string(ERR_get_error(), nullptr));
        return;
    }

    // 创建 BIO
    m_readBio = BIO_new(BIO_s_mem());
    m_writeBio = BIO_new(BIO_s_mem());

    if (!m_readBio || !m_writeBio) {
        LOG_ERROR("Failed to create BIO objects");
        SSL_free(m_ssl);
        m_ssl = nullptr;
        return;
    }

    SSL_set_bio(m_ssl, m_readBio, m_writeBio);
    SSL_set_accept_state(m_ssl);  // 设置为服务器模式    

    // 设置 SSL 选项
    SSL_set_mode(m_ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    SSL_set_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

    // 设置连接回调
    m_conn->setMessageCallback(
        std::bind(&SSLConnection::onRead, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3));
}

SSLConnection::~SSLConnection() {
    if (m_ssl) {
        SSL_free(m_ssl);  // 这会同时释放 BIO
        m_ssl = nullptr;
    }
}

void SSLConnection::startHandshake() {
    SSL_set_accept_state(m_ssl);
    handleHandshake();
}

void SSLConnection::send(const void* data, size_t len) {
    if (m_state != SSLState::ESTABLISHED) {
        LOG_ERROR("Cannot send data before SSL handshake is complete");
        return;
    }
    
    int written = SSL_write(m_ssl, data, len);
    if (written <= 0) {
        int err = SSL_get_error(m_ssl, written);
        LOG_ERROR( "SSL_write failed: %s", ERR_error_string(err, nullptr));
        return;
    }
    
    char buf[4096];
    int pending;
    while ((pending = BIO_pending(m_writeBio)) > 0) {
        int bytes = BIO_read(m_writeBio, buf, 
                           std::min(pending, static_cast<int>(sizeof(buf))));
        if (bytes > 0) {
            m_conn->send(std::string(buf, bytes));
        }
    }
}

void SSLConnection::onRead(const TcpConnectionPtr& conn, BufferPtr buf, lemon::base::Timestamp time) {
    if (m_state == SSLState::HANDSHAKE) {
        // 将数据写入BIO
        BIO_write(m_readBio, buf->peek(), buf->readableBytes());
        buf->retrieve(buf->readableBytes());
        handleHandshake();
    } else if (m_state == SSLState::ESTABLISHED) {
        // 解密数据
        char decryptedData[4096];
        int ret = SSL_read(m_ssl, decryptedData, sizeof(decryptedData));
        if (ret > 0) {
            // 创建新的 Buffer 存储解密后的数据
            lemon::net::Buffer decryptedBuffer;
            decryptedBuffer.append(decryptedData, ret);
            // 调用上层回调处理解密后的数据
            if (m_messageCallback) {
                m_messageCallback(conn, &decryptedBuffer, time);
            }
        }
    }
}

// SSL BIO 操作回调
int SSLConnection::bioWrite(BIO* bio, const char* data, int len) {
    (void)bio; (void)data; (void)len;
    return 0;
}

int SSLConnection::bioRead(BIO* bio, char* data, int len) {
    (void)bio; (void)data; (void)len;
    return 0;
}

long SSLConnection::bioCtrl(BIO* bio, int cmd, long num, void* ptr) {
    (void)bio; (void)cmd; (void)num; (void)ptr;
    return 0;
}

void SSLConnection::handleHandshake() {
    int ret = SSL_do_handshake(m_ssl);
    if (ret == 1) {
        // 握手成功
        m_state = SSLState::ESTABLISHED;
        LOG_INFO("SSL handshake completed successfully");
        LOG_INFO("Using cipher: %s", SSL_get_cipher(m_ssl));
        LOG_INFO("Protocol version: %s", SSL_get_version(m_ssl));

        if (!m_messageCallback) {
            LOG_WARN("No message callback set after SSL handshake");
        }
        return;
    }

    int err = SSL_get_error(m_ssl, ret);
    switch (err) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            // 正常的握手过程，需要继续
            break;
        default:
            // 获取详细的错误信息
            char errBuf[256];
            unsigned long errCode = ERR_get_error();
            ERR_error_string_n(errCode, errBuf, sizeof(errBuf));
            LOG_ERROR("SSL handshake failed: %s", errBuf);
            m_conn->shutdown();  // 关闭连接
            break;
    }
}

void SSLConnection::onEncrypted(const char* data, size_t len) {
    (void)data; (void)len;
}

void SSLConnection::onDecrypted(const char* data, size_t len) {
    (void)data; (void)len;
}

SSLError SSLConnection::getLastError(int ret) {
    (void)ret;
    return SSLError::NONE;
}

void SSLConnection::handleError(SSLError error) {
    (void)error;
}