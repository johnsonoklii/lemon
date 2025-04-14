#ifndef LEMON_HTTP_ROUTERHANDLER_H
#define LEMON_HTTP_ROUTERHANDLER_H

namespace lemon {
namespace http {
namespace router {

class HttpRequest;
class HttpResponse;

class RouterHandler {
public:
    virtual ~RouterHandler() = default;

    virtual void handle(const HttpRequest& request, HttpResponse& response) = 0;
};  

} // namespace router
} // namespace http
} // namespace lemon

#endif