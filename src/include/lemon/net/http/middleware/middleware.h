#ifndef LEMON_HTTP_MIDDLEWARE_HPP
#define LEMON_HTTP_MIDDLEWARE_HPP

#include "lemon/net/http/http_request.h"
#include "lemon/net/http/http_response.h"

#include <memory>

namespace lemon {
namespace http {
namespace middleware {

class Middleware {
public:
    virtual ~Middleware() = default;

    // 请求前处理
    virtual void before(HttpRequest& request) = 0;
    
    // 响应后处理
    virtual void after(HttpResponse& response) = 0;

    void setNext(std::shared_ptr<Middleware> next) {
        m_next_middleware = next;
    }
    
protected:
    std::shared_ptr<Middleware> m_next_middleware;
};

} // namespace middleware
} // namespace http
} // namespace lemon

#endif