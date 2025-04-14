#ifndef LEMON_HTTP_ROUTER_H
#define LEMON_HTTP_ROUTER_H

#include "lemon/net/http/http_request.h"
#include "lemon/net/http/http_response.h"
#include "lemon/net/http/router/router_handler.h"

#include <unordered_map>
#include <memory>
#include <functional>
#include <regex>

namespace lemon {
namespace http {
namespace router {

class Router {
public:
    using HandlerPtr = std::shared_ptr<RouterHandler>;
    using HandlerCallback = std::function<void(const HttpRequest &, HttpResponse*)>;

    // 路由键（请求方法 + URI）
    struct RouteKey {
        HttpRequest::Method method;
        std::string path;

        bool operator==(const RouteKey &other) const {
            return method == other.method && path == other.path;
        }
    };

    // 为RouteKey 定义哈希函数
    struct RouteKeyHash {
        size_t operator()(const RouteKey &key) const
        {
            size_t methodHash = std::hash<int>{}(static_cast<int>(key.method));
            size_t pathHash = std::hash<std::string>{}(key.path);
            return methodHash * 31 + pathHash;
        }
    };

public:
    void registerHandler(HttpRequest::Method method, const std::string &path, const HandlerPtr& handler);
    void registerCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback);
    
    // 注册动态路由
    void addRegexHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler);
    void addRegexCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback);

    bool route(const HttpRequest &req, HttpResponse *resp);
private:
    std::regex convertToRegex(const std::string &pathPattern);
    void extractPathParameters(const std::smatch &match, HttpRequest &request);

private:
    struct RouteHandlerObj {
        HttpRequest::Method method_;
        std::regex pathRegex_;
        HandlerPtr handler_;
        RouteHandlerObj(HttpRequest::Method method, std::regex pathRegex, HandlerPtr handler)
            : method_(method), pathRegex_(pathRegex), handler_(handler) {}
    };

    struct RouteCallbackObj {
        HttpRequest::Method method_;
        std::regex pathRegex_;
        HandlerCallback callback_;
        RouteCallbackObj(HttpRequest::Method method, std::regex pathRegex, const HandlerCallback &callback)
            : method_(method), pathRegex_(pathRegex), callback_(callback) {}
    };

    std::unordered_map<RouteKey, HandlerPtr, RouteKeyHash>      m_handlers;          // 精准匹配
    std::unordered_map<RouteKey, HandlerCallback, RouteKeyHash> m_callbacks;         // 精准匹配
    std::vector<RouteHandlerObj>                                m_regexHandlers;     // 正则匹配
    std::vector<RouteCallbackObj>                               m_regexCallbacks;    // 正则匹配
};

} // namespace router
} // namespace http
} // namespace lemon

#endif