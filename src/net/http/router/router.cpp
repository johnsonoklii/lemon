#include "lemon/net/http/router/router.h"
#include "lemon/net/http/http_request.h"

using namespace lemon::http::router;
using namespace lemon::http;

// FIXME: 是否适合使用 const shared_ptr&
void Router::registerHandler(HttpRequest::Method method, const std::string &path, const HandlerPtr& handler) {
    RouteKey key{method, path};
    m_handlers[key] = std::move(handler);
}

void Router::registerCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback) {
    RouteKey key{method, path};
    m_callbacks[key] = std::move(callback);
}

void Router::addRegexHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler) {
    std::regex pathRegex = convertToRegex(path);
    m_regexHandlers.emplace_back(method, pathRegex, handler);
}

void Router::addRegexCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback) {
    std::regex pathRegex = convertToRegex(path);
    m_regexCallbacks.emplace_back(method, pathRegex, callback);
}

bool Router::route(const HttpRequest &req, HttpResponse *resp) {
    RouteKey key{req.method(), req.path()};

    // 处理器
    auto handlerIt = m_handlers.find(key);
    if (handlerIt != m_handlers.end()) {
        handlerIt->second->handle(req, resp);
        return true;
    }

    // 回调
    auto callbackIt = m_callbacks.find(key);
    if (callbackIt != m_callbacks.end()) {
        callbackIt->second(req, resp);
        return true;
    }

    // 动态路由处理器
    for (const auto& it : m_regexHandlers) {
        std::smatch match;
        std::string pathStr(req.path());
        if (it.method_ == req.method() && std::regex_match(pathStr, match, it.pathRegex_)) {
            HttpRequest newReq(req);
            extractPathParameters(match, newReq);
            it.handler_->handle(newReq, resp);
            return true;
        }
    }

    for (const auto& it : m_regexCallbacks) {
        std::smatch match;
        std::string pathStr(req.path());
        if (it.method_ == req.method() && std::regex_match(pathStr, match, it.pathRegex_)) {
            HttpRequest newReq(req);
            extractPathParameters(match, newReq);
            it.callback_(newReq, resp);
            return true;
        }
    }

    return false;
}

std::regex Router::convertToRegex(const std::string &pathPattern) {
    // path: "/user/:id/profile"; 将 /:id 替换成 [^/]+
    // regexPattern = "^/user/([^/]+/login$"
    std::string regexPattern = "^" + std::regex_replace(pathPattern, std::regex(R"(/:([^/]+))"), R"(/([^/]+))") + "$";
    return std::regex(regexPattern);
}

void Router::extractPathParameters(const std::smatch &match, HttpRequest &request) {
    for (size_t i = 1; i < match.size(); ++i){
        request.setPathParam("param" + std::to_string(i), match[i].str());
    }
}