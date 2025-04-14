#ifndef LEMON_HTTP_CONTEXT_H
#define LEMON_HTTP_CONTEXT_H

#include "lemon/net/http/http_request.h"
#include "lemon/net/buffer.h"
#include "lemon/base/timestamp.h"

namespace lemon {
namespace http {

class HttpContext {
public:
    enum HttpRequestParseState
    {
        kExpectRequestLine, // 解析请求行
        kExpectHeaders,     // 解析请求头
        kExpectBody,        // 解析请求体
        kGotAll,            // 解析完成
    };

    HttpContext();

    bool parseRequest(lemon::net::Buffer* buf, lemon::base::Timestamp receiveTime);
    bool gotAll() const {  return m_state == kGotAll; }

    void reset();

    const HttpRequest& request() const { return m_request; }
    HttpRequest& request() { return m_request; }

private:    
    bool parseRequestLine(lemon::net::Buffer* buf, lemon::base::Timestamp receiveTime, bool& hasMore);
    bool parseHeaders(lemon::net::Buffer* buf, bool& hasMore);
    bool parseBody(lemon::net::Buffer* buf, bool& hasMore);

    HttpRequestParseState m_state;
    HttpRequest m_request;
};

} // namespace http
} // namespace lemon

#endif