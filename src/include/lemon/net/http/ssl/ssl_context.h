#ifndef LEMON_SSL_CONTEXT_H
#define LEMON_SSL_CONTEXT_H

#include "lemon/net/http/ssl/ssl_config.h"

#include <openssl/ssl.h>

namespace lemon {
namespace http {
namespace ssl {

class SSLContext {
public:
    explicit SSLContext(const SSLConfig& config);
    ~SSLContext();

    bool initialize();
    SSL_CTX* getNativeHandle() { return m_ctx; }

private:
    bool loadCertificates();
    bool setupProtocol();
    void setupSessionCache();
    static void handleSslError(const char* msg);

private:
    SSL_CTX*  m_ctx; // SSL上下文
    SSLConfig m_config; // SSL配置
};

}
}
}

#endif