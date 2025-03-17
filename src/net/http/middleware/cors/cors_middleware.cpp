#include "lemon/net/http/middleware/cors/cors_middleware.h"
#include "lemon/base/logger/logger.h"

#include <algorithm>

using namespace lemon::http::middleware;
using namespace lemon::log;

CorsMiddleware::CorsMiddleware(const CorsConfig& config): m_config(config) {}

void CorsMiddleware::before(HttpRequest& request) {
    LOG_DEBUG("CorsMiddleware::before....]\n");
    if (request.method() == HttpRequest::Method::kOptions) {
        LOG_INFO("Processing CORS preflight request\n");
        HttpResponse response;
        handlePreflightRequest(request, response);
        throw response;
    }
}

void CorsMiddleware::after(HttpResponse& response) {
    LOG_DEBUG("CorsMiddleware::after....]\n");
    if (!m_config.allowedOrigins.empty()) {
        if (std::find(m_config.allowedOrigins.begin(), m_config.allowedOrigins.end(), "*") != m_config.allowedOrigins.end()) {
            addCorsHeaders(response, "*");
        } else {
            // 添加第一个允许的源
            addCorsHeaders(response, m_config.allowedOrigins[0]);
        }
    }
}

bool CorsMiddleware::isOriginAllowed(const std::string& origin) const {
    return m_config.allowedOrigins.empty() 
            || std::find(m_config.allowedOrigins.begin(), m_config.allowedOrigins.end(), "*") != m_config.allowedOrigins.end()
            || std::find(m_config.allowedOrigins.begin(), m_config.allowedOrigins.end(), origin) != m_config.allowedOrigins.end();
}

void CorsMiddleware::handlePreflightRequest(const HttpRequest& request, HttpResponse& response) {
    const std::string& origin = request.getHeader("Origin");
        
    if (!isOriginAllowed(origin)) {
        LOG_WARN("Origin not allowed: %s\n", origin.c_str());
        response.setStatusCode(HttpResponse::k403Forbidden);
        return;
    }
    addCorsHeaders(response, origin);
    response.setStatusCode(HttpResponse::k204NoContent);
    LOG_INFO("Preflight request processed successfully\n");
}

void CorsMiddleware::addCorsHeaders(HttpResponse& response, const std::string& origin) {
    try {
        response.addHeader("Access-Control-Allow-Origin", origin);
        
        if (m_config.allowCredentials) {
            response.addHeader("Access-Control-Allow-Credentials", "true");
        }
        
        if (!m_config.allowedMethods.empty()) {
            response.addHeader("Access-Control-Allow-Methods", 
                             join(m_config.allowedMethods, ", "));
        }
        
        if (!m_config.allowedHeaders.empty()) {
            response.addHeader("Access-Control-Allow-Headers", 
                             join(m_config.allowedHeaders, ", "));
        }
        
        response.addHeader("Access-Control-Max-Age", 
                          std::to_string(m_config.maxAge));
        
        LOG_DEBUG("CORS headers added successfully\n");
    } 
    catch (const std::exception& e) {
        LOG_ERROR("Error adding CORS headers: %s\n", e.what());
    }
}

std::string CorsMiddleware::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    std::ostringstream result;
    for (size_t i = 0; i < strings.size(); ++i) 
    {
        if (i > 0) result << delimiter;
        result << strings[i];
    }
    return result.str();
}