#ifndef LEMON_HTTP_RESPONSE_H
#define LEMON_HTTP_RESPONSE_H

#include "lemon/net/buffer.h"

#include <string>
#include <unordered_map>

namespace lemon {
namespace http {

class HttpResponse {
public:
    enum HttpStatusCode
    {
        kUnknown,
        k200Ok = 200,
        k204NoContent = 204,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k401Unauthorized = 401,
        k403Forbidden = 403,
        k404NotFound = 404,
        k409Conflict = 409,
        k500InternalServerError = 500,
    };

    HttpResponse(bool close = true);

    void setVersion(const std::string& version) { m_http_version = version; }

    void setStatusCode(HttpStatusCode code) { m_status_code = code; }
    void setBody(const std::string& body) { m_body = body; }
    void setStatusMessage(const std::string& message) { m_status_message = message; }
    void setCloseConnection(bool close) { m_close = close; }
    bool closeConnection() const { return m_close; }

    void setContentType(const std::string& content_type) {
        m_headers["Content-Type"] = content_type;
    }
    
    void setContentLength(size_t length) {
        m_headers["Content-Length"] = std::to_string(length);
    }

    void addHeader(const std::string& key, const std::string& value) {
        m_headers[key] = value;
    }

    void setStatusLine(const std::string& version,
                        HttpStatusCode statusCode,
                        const std::string& statusMessage);


    void appendToBuffer(lemon::net::Buffer* buf) const;


private:
    std::string m_http_version;
    HttpStatusCode m_status_code;
    std::string m_status_message;   
    std::string m_body;
    std::unordered_map<std::string, std::string> m_headers;
    bool m_close;
    bool m_is_file;
};

} // namespace http
} // namespace lemon    

#endif