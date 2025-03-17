#ifndef LEMON_HTTP_SERVER_H
#define LEMON_HTTP_SERVER_H

#include "lemon/net/tcp_server.h"
#include "lemon/base/nocopyable.h"
#include "lemon/net/http/router/router.h"
#include "lemon/net/http/middleware/middleware_chain.h"
#include "lemon/net/http/session/session_manager.h"
#include "lemon/net/http/session/session_storage.h"
#include "lemon/net/http/session/session.h"
#include "lemon/net/http/ssl/ssl_connection.h"

namespace lemon {
namespace http {

class HttpRequest;
class HttpResponse;

class HttpServer: lemon::base::noncopyable {
public:
    using HttpCallback = std::function<void(const http::HttpRequest&, http::HttpResponse*)>;

    HttpServer() = default;
    HttpServer(std::string ip, uint16_t port
                , const std::string& name
                , lemon::net::TcpServer::Option option = lemon::net::TcpServer::kReusePort);

    void setThreadNum(int numThreads) {  m_server.setThreadNum(numThreads); }
    void setHttpCallback(const HttpCallback& cb) { m_httpCallback = cb; }

    void start();

    void setSessionManager(session::SessionManager* manager) {
        m_sessionManager = manager;
    }

    session::SessionManager* sessionManager() {
        return m_sessionManager;
    }

    // 注册静态路由
    void Get(const std::string& path, const HttpCallback& cb) {
        m_router.registerCallback(HttpRequest::kGet, path, cb);
    }

    void Get(const std::string& path, router::Router::HandlerPtr handler) {
        m_router.registerHandler(HttpRequest::kGet, path, handler);
    }

    void Post(const std::string& path, const HttpCallback& cb) {
        m_router.registerCallback(HttpRequest::kPost, path, cb);
    }

    void Post(const std::string& path, router::Router::HandlerPtr handler) {
        m_router.registerHandler(HttpRequest::kPost, path, handler);
    }

    // 添加动态路由
    void addRoute(HttpRequest::Method method, const std::string& path, router::Router::HandlerPtr handler) {
        m_router.addRegexHandler(method, path, handler);
    }

    void addRoute(HttpRequest::Method method, const std::string& path, const router::Router::HandlerCallback& callback) {
        m_router.addRegexCallback(method, path, callback);
    }

    void addMiddleware(std::shared_ptr<middleware::Middleware> middleware) {
        m_middlewareChain.add(middleware);
    }

    void enableSSL(bool enable)  {
        m_useSSL = enable;
    }

    void setSslConfig(const ssl::SSLConfig& config);

    lemon::net::EventLoop* getLoop() const { return m_server.getLoop(); }

private:
    void init();
    void onConnection(const lemon::net::TcpConnectionPtr& conn);
    void onMessage(const lemon::net::TcpConnectionPtr& conn,
                   lemon::net::Buffer* buf,
                   lemon::base::Timestamp receiveTime);
    void onRequest(const lemon::net::TcpConnectionPtr& conn, const HttpRequest& req);
    
    void handleRequest(const HttpRequest& req, HttpResponse* resp);

private:
    const std::string HttpContextKey = "HttpContext";
    lemon::net::InetAddress m_address;
    lemon::net::EventLoop m_mainLoop;
    lemon::net::TcpServer m_server;
    HttpCallback m_httpCallback;

    router::Router m_router;
    middleware::MiddlewareChain m_middlewareChain;
    session::SessionManager* m_sessionManager;
    
    bool m_useSSL;
    std::unique_ptr<ssl::SSLContext>             m_sslCtx; // SSL 上下文
    std::map<lemon::net::TcpConnectionPtr, std::unique_ptr<ssl::SSLConnection>> m_sslConns;
};

} // namespace http
} // namespace lemon

#endif