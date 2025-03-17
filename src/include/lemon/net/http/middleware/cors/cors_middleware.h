#ifndef LEMON_HTTP_CORS_MIDDLEWARE_H
#define LEMON_HTTP_CORS_MIDDLEWARE_H

#include "lemon/net/http/middleware/middleware.h"
#include "lemon/net/http/middleware/cors/cors_config.h"

#include <string>

namespace lemon {
namespace http {
namespace middleware {


class CorsMiddleware : public Middleware {
public:
    explicit CorsMiddleware(const CorsConfig& config = CorsConfig::defaultConfig());

    void before(HttpRequest& request) override;
    void after(HttpResponse& response) override;

    std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

private:
    bool isOriginAllowed(const std::string& origin) const;
    void handlePreflightRequest(const HttpRequest& request, HttpResponse& response);
    void addCorsHeaders(HttpResponse& response, const std::string& origin);

private:
    CorsConfig m_config;
};

} // namespace middleware
} // namespace http
} // namespace lemon

#endif