#include "lemon/net/http/middleware/middleware_chain.h"

#include "lemon/base/logger/logger.h"

using namespace lemon::http::middleware;
using namespace lemon::log;

void MiddlewareChain::add(std::shared_ptr<Middleware> middleware) {
    m_middlewares.push_back(middleware);
}

void MiddlewareChain::processBefore(HttpRequest& request) {
    for (auto& middleware : m_middlewares) {
        middleware->before(request);
    }
}

void MiddlewareChain::processAfter(HttpResponse& response) {
    try {
        for (auto it = m_middlewares.rbegin(); it != m_middlewares.rend(); ++it) {
            if (*it) {
                (*it)->after(response);
            }
        }
    } catch (const std::exception& e) {
        LOG_INFO("processAfter exception: %s", e.what());
    }
}