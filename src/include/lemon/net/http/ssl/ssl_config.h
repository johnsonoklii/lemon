#ifndef LEMON_SSL_CONFIG_H
#define LEMON_SSL_CONFIG_H

#include "lemon/net/http/ssl/ssl_types.h"

#include <string>

namespace lemon {
namespace http {
namespace ssl {

class SSLConfig {
public:
    SSLConfig();
    ~SSLConfig() = default;

    // 证书配置
    void setCertificateFile(const std::string& certFile) { m_certFile = certFile; }
    void setPrivateKeyFile(const std::string& keyFile) { m_keyFile = keyFile; }
    void setCertificateChainFile(const std::string& chainFile) { m_chainFile = chainFile; }
    
    // 协议版本和加密套件配置
    void setProtocolVersion(SSLVersion version) { m_version = version; }
    void setCipherList(const std::string& cipherList) { m_cipherList = cipherList; }
    
    // 客户端验证配置
    void setVerifyClient(bool verify) { m_verifyClient = verify; }
    void setVerifyDepth(int depth) { m_verifyDepth = depth; }
    
    // 会话配置
    void setSessionTimeout(int seconds) { m_sessionTimeout = seconds; }
    void setSessionCacheSize(long size) { m_sessionCacheSize = size; }

    // Getters
    const std::string& getCertificateFile() const { return m_certFile; }
    const std::string& getPrivateKeyFile() const { return m_keyFile; }
    const std::string& getCertificateChainFile() const { return m_chainFile; }
    SSLVersion getProtocolVersion() const { return m_version; }
    const std::string& getCipherList() const { return m_cipherList; }
    bool getVerifyClient() const { return m_verifyClient; }
    int getVerifyDepth() const { return m_verifyDepth; }
    int getSessionTimeout() const { return m_sessionTimeout; }
    long getSessionCacheSize() const { return m_sessionCacheSize; }

private:
    std::string m_certFile;         // 证书文件
    std::string m_keyFile;          // 私钥文件
    std::string m_chainFile;        // 证书链文件
    SSLVersion  m_version;          // 协议版本
    std::string m_cipherList;       // 加密套件
    bool        m_verifyClient;     // 是否验证客户端
    int         m_verifyDepth;      // 验证深度
    int         m_sessionTimeout;   // 会话超时时间
    long        m_sessionCacheSize; // 会话缓存大小
};

} // namespace ssl
} // namespace http
} // namespace lemon


#endif