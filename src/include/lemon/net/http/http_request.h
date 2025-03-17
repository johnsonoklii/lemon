#ifndef LEMON_HTTP_REQUEST_H
#define LEMON_HTTP_REQUEST_H

#include "lemon/base/timestamp.h"

#include <string>
#include <map>
#include <unordered_map>
#include <vector>

namespace lemon {
namespace http {

class HttpRequest {
public:
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete, kOptions
    };

    HttpRequest();
    void setReceiveTime(lemon::base::Timestamp t) { m_receive_time = t; }
    lemon::base::Timestamp receiveTime() const { return m_receive_time; }
    
    bool setMethod(const std::string& m);
    Method method() const { return m_method; }
    
    void setPath(const std::string& path) { m_path = path; }
    std::string path() const { return m_path; }

    void setVersion(const std::string& v) { m_version = v; }
    std::string version() const { return m_version; }

    // 从问号后面分割参数
    void setPathParam(const std::string& key, const std::string& value) {
        m_path_params[key] = value;
    }
    std::string pathParam(const std::string& key) const;

    void setQueryParams(const std::string& path);
    std::vector<std::string> queryParam(const std::string& key) const;

    void addHeader(const std::string& key, const std::string& value) { m_headers[key] = value; }
    std::string getHeader(const std::string& key) const;

    const std::unordered_map<std::string, std::string>& headers() const { return m_headers; }

    void setBody(const std::string& body) { m_body = body; }
    std::string body() const { return m_body; }

    void setContentLength(uint64_t length) { m_content_length = length; }
    uint64_t contentLength() const { return m_content_length; }
    
    void swap(HttpRequest& other);

private:
    using QueryParamMap = std::unordered_map<std::string, std::vector<std::string>>;
    Method m_method;
    std::string m_version;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_path_params;
    QueryParamMap m_query_params;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
    uint64_t m_content_length;
    lemon::base::Timestamp m_receive_time;
};

} // namespace http
} // namespace lemon

#endif