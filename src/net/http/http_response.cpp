#include "lemon/net/http/http_response.h"

using namespace lemon;
using namespace lemon::http; 

HttpResponse::HttpResponse(bool close)
: m_status_code(kUnknown)
 , m_close(close)
 , m_is_file(false) {

}

void HttpResponse::setStatusLine(const std::string& version,
                                    HttpStatusCode statusCode,
                                    const std::string& statusMessage) {
    m_http_version = version;
    m_status_code = statusCode;
    m_status_message = statusMessage;
}

void HttpResponse::appendToBuffer(lemon::net::Buffer* outputBuf) const {
    (void)outputBuf;

    char buf[128];
    int len = snprintf(buf, sizeof buf, "%s %d ", m_http_version.data(), m_status_code);

    // status line
    outputBuf->append(buf, len);
    outputBuf->append(m_status_message.data(), m_status_message.size());
    outputBuf->append("\r\n", 2);

    if (m_close) {
        outputBuf->append("Connection: close\r\n", 19);
    } else {
        outputBuf->append("Connection: Keep-Alive\r\n", 24);
    }

    for (const auto& header : m_headers) {
        outputBuf->append(header.first.data(), header.first.size());
        outputBuf->append(": ", 2);
        outputBuf->append(header.second.data(), header.second.size());
        outputBuf->append("\r\n", 2);
    }


    outputBuf->append("\r\n", 2);
    outputBuf->append(m_body.data(), m_body.size());
}


