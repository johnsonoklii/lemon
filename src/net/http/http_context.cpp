#include "lemon/net/http/http_context.h"
#include "lemon/base/utils.h"

#include "lemon/base/logger/logger.h"

using namespace lemon;
using namespace lemon::http;
using namespace lemon::base;
using namespace lemon::log;

HttpContext::HttpContext(): m_state(kExpectRequestLine) {}

bool HttpContext::parseRequest(lemon::net::Buffer* buf, lemon::base::Timestamp receiveTime) {
    bool ok = true; // 是否解析成功
    bool hasMore = true; // 是否还有待解析的数据
    while (hasMore) {
        if (m_state == kExpectRequestLine) {
            ok = parseRequestLine(buf, receiveTime, hasMore);
        } else if (m_state == kExpectHeaders) {
            ok = parseHeaders(buf, hasMore);
        } else if (m_state == kExpectBody) {
            ok = parseBody(buf, hasMore);
        } else if (m_state == kGotAll) {
            ok = true;
            hasMore = false;
        }

        if (!ok) {
           hasMore = false;     
        }
    }

    return ok;
}

bool HttpContext::parseRequestLine(lemon::net::Buffer* buf, lemon::base::Timestamp receiveTime, bool& hasMore) {
    const char* crlf = buf->findCRLF();
    if (!crlf) {
        hasMore = false;
        return true;
    }

    const char* start = buf->peek();
    const char* end = crlf;
    const char* space = std::find(start, end, ' ');

    bool succeed = true;

    // 1. 请求方法
    if (space != end && m_request.setMethod(std::string(start, space))) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char* argument_start = std::find(start, space, '?');
            if (argument_start != space) {
                // 带请求参数，2. 请求路径
                m_request.setPath(std::string(start, argument_start));
                m_request.setQueryParams(std::string(argument_start + 1, space));
            } else {
                // 不带请求参数
                m_request.setPath(std::string(start, space));
            }

            // 3. 版本
            start = space + 1;
            std::string version(start, end);
            if (version != "HTTP/1.1" && version != "HTTP/1.0") {
                succeed = false;
            } else {
                m_request.setVersion(version);
            }
        }    
    }

    if (succeed) {
        buf->retrieveUntil(crlf + 2);
        m_request.setReceiveTime(receiveTime);
        m_state = kExpectHeaders;
    }

    return succeed;
}

bool HttpContext::parseHeaders(lemon::net::Buffer* buf, bool& hasMore) {
    const char *crlf = buf->findCRLF();
    if (!crlf) {
        hasMore = false;
        return true;
    }

    const char* end = crlf;

    const char* colon = std::find(buf->peek(), end, ':');
    if (colon < end) {
        std::string key(buf->peek(), colon);
        std::string value(colon + 1, end);

        key = StringUtil::trim(key);
        value = StringUtil::trim(value);
        
        m_request.addHeader(key, value);
    } else if (buf->peek() == end) {
        // 空行
        if (m_request.method() == HttpRequest::kPost ||
            m_request.method() == HttpRequest::kPut) {
            std::string content_length = m_request.getHeader("Content-Length");
            if (content_length.empty()) {
                // POST/PUT 请求没有 Content-Length，是HTTP语法错误
                return false;
            }

            m_request.setContentLength(std::stoi(content_length));
            if (m_request.contentLength() > 0) {
                m_state = kExpectBody;
            } else {
                m_state = kGotAll;
            }
        } else {
            // GET
            m_state = kGotAll;
        }
    } else {
        return false;
    }

    buf->retrieveUntil(crlf + 2);

    return true;
}

bool HttpContext::parseBody(lemon::net::Buffer* buf, bool& hasMore) {
    if (buf->readableBytes() < m_request.contentLength()) {
        hasMore = false;
        return true;
    }

    std::string body = buf->retrieveAsString(m_request.contentLength());
    m_request.setBody(body);

    m_state = kGotAll;
    return true;
}

void HttpContext::reset() {
    m_state = kExpectRequestLine;
    HttpRequest dummyDate;
    m_request.swap(dummyDate);
}