#ifndef LEMON_HTTP_MIDDLEWARE_CHAIN_H
#define LEMON_HTTP_MIDDLEWARE_CHAIN_H

#include "lemon/net/http/middleware/middleware.h"

#include <memory>
#include <vector>

namespace lemon {
namespace http {
namespace middleware {

class MiddlewareChain {
public:
    void add(std::shared_ptr<Middleware> middleware);
    void processBefore(HttpRequest& request);
    void processAfter(HttpResponse& response);
private:
    std::vector<std::shared_ptr<Middleware>> m_middlewares;
};

} // namespace middleware
} // namespace http
} // namespace lemon

#endif