#include "lemon/net/http/ssl/ssl_config.h"

using namespace lemon::http::ssl;

SSLConfig::SSLConfig()
: m_version(SSLVersion::TLS_1_2)
 , m_cipherList("HIGH:!aNULL:!MDS")
 , m_verifyClient(false)
 , m_verifyDepth(4)
 , m_sessionTimeout(300)
 , m_sessionCacheSize(20480L)
{
}