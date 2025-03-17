#include "lemon/net/http/ssl/ssl_context.h"
#include "lemon/base/logger/logger.h"

#include <openssl/err.h>

using namespace lemon::http::ssl;
using namespace lemon::log;

SSLContext::SSLContext(const SSLConfig& config)
: m_ctx(nullptr)
 , m_config(config) {

}

SSLContext::~SSLContext() {
    if (m_ctx) {
        SSL_CTX_free(m_ctx);
    }
}

bool SSLContext::initialize() {
    // 初始化 OpenSSL
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | 
        OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);

    // 创建 SSL 上下文
    const SSL_METHOD* method = TLS_server_method();
    m_ctx = SSL_CTX_new(method);
    if (!m_ctx) {
        handleSslError("SSL_CTX_new");
        return false;
    }

     // 设置 SSL 选项
     long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | 
     SSL_OP_NO_COMPRESSION |
     SSL_OP_CIPHER_SERVER_PREFERENCE;
    SSL_CTX_set_options(m_ctx, options);

    // 加载证书和私钥
    if (!loadCertificates()) {
        handleSslError("loadCertificates");
        return false;
    }

    // 设置协议版本
    if (!setupProtocol()) {
        return false;
    }

    // 设置会话缓存
    setupSessionCache();

    LOG_INFO("SSL context initialized successfully");
    return true;
}

bool SSLContext::loadCertificates() {
    // 加载证书
    if (SSL_CTX_use_certificate_file(m_ctx,
        m_config.getCertificateFile().c_str(), SSL_FILETYPE_PEM) <= 0) {
        handleSslError("Failed to load server certificate");
        return false;
    }

    // 加载私钥
    if (SSL_CTX_use_PrivateKey_file(m_ctx, 
        m_config.getPrivateKeyFile().c_str(), SSL_FILETYPE_PEM) <= 0) {
        handleSslError("Failed to load private key");
        return false;
    }

    // 验证私钥
    if (!SSL_CTX_check_private_key(m_ctx)) {
        handleSslError("Private key does not match the certificate");
        return false;
    }

    // 加载证书链
    if (!m_config.getCertificateChainFile().empty()) {
        if (SSL_CTX_use_certificate_chain_file(m_ctx,
            m_config.getCertificateChainFile().c_str()) <= 0) {
            handleSslError("Failed to load certificate chain");
            return false;
        }
    }

    return true;
}

bool SSLContext::setupProtocol() {
    // 设置 SSL/TLS 协议版本
    long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
    switch (m_config.getProtocolVersion()) {
        case SSLVersion::TLS_1_0:
            options |= SSL_OP_NO_TLSv1;
            break;
        case SSLVersion::TLS_1_1:
            options |= SSL_OP_NO_TLSv1_1;
            break;
        case SSLVersion::TLS_1_2:
            options |= SSL_OP_NO_TLSv1_2;
            break;
        case SSLVersion::TLS_1_3:
            options |= SSL_OP_NO_TLSv1_3;
            break;
    }
    SSL_CTX_set_options(m_ctx, options);

    // 设置加密套件
    if (!m_config.getCipherList().empty()) {
        if (SSL_CTX_set_cipher_list(m_ctx,
            m_config.getCipherList().c_str()) <= 0) {
            handleSslError("Failed to set cipher list");
            return false;
        }
    }

    return true;
}

void SSLContext::setupSessionCache() {
    SSL_CTX_set_session_cache_mode(m_ctx, SSL_SESS_CACHE_SERVER);
    SSL_CTX_sess_set_cache_size(m_ctx, m_config.getSessionCacheSize());
    SSL_CTX_set_timeout(m_ctx, m_config.getSessionTimeout());
}

void SSLContext::handleSslError(const char* msg) {
    char buf[256];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    LOG_ERROR("SSL error: %s, %s", msg, buf);
}