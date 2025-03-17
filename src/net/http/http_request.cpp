#include "lemon/net/http/http_request.h"

#include <sstream>

#include <assert.h>

using namespace lemon;
using namespace lemon::http;

HttpRequest::HttpRequest()
: m_method(Method::kInvalid)
 , m_version("Unknown")
 , m_path()
 , m_body()
 , m_content_length(0)
 , m_receive_time() {

}

bool HttpRequest::setMethod(const std::string& m) {
    assert(m_method == Method::kInvalid);

    if (m == "GET") {
        m_method = kGet;
    } else if (m == "POST") {
        m_method = kPost;
    } else if (m == "PUT") {
        m_method = kPut;
    } else if (m == "DELETE") {
        m_method = kDelete;
    } else if (m == "OPTIONS") {
        m_method = kOptions;
    } else {
        m_method = kInvalid;
    }

    return m_method != kInvalid;
}

std::string HttpRequest::pathParam(const std::string& key) const {
    return m_path_params.count(key) ? m_path_params.at(key) : "";
}

void HttpRequest::setQueryParams(const std::string& path) {
    std::istringstream iss(path);
    std::string pair;
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            m_query_params[key].push_back(std::move(value));
        }
    }
}

std::vector<std::string> HttpRequest::queryParam(const std::string& key) const {
    return m_query_params.count(key) ? m_query_params.at(key) : std::vector<std::string>();
}

std::string HttpRequest::getHeader(const std::string& key) const {
    return m_headers.count(key) ? m_headers.at(key) : "";
}

void HttpRequest::swap(HttpRequest& other) {
    std::swap(m_method, other.m_method);
    std::swap(m_version, other.m_version);
    std::swap(m_path, other.m_path);
    std::swap(m_path_params, other.m_path_params);
    std::swap(m_query_params, other.m_query_params);
    std::swap(m_headers, other.m_headers);
    std::swap(m_body, other.m_body);
    std::swap(m_content_length, other.m_content_length);
    std::swap(m_receive_time, other.m_receive_time);
}
