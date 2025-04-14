#ifndef LEMON_HTTP_ROUTERHANDLER_H
#define LEMON_HTTP_ROUTERHANDLER_H

#include "lemon/net/http/http_request.h"
#include "lemon/net/http/http_response.h"

namespace lemon {
namespace http {
namespace router {

class RouterHandler {
public:
    virtual ~RouterHandler() = default;

    virtual void handle(const HttpRequest& request, HttpResponse* response) = 0;
};  

} // namespace router
} // namespace http
} // namespace lemon

#endif