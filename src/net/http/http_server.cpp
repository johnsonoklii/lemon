#include "lemon/net/http/http_server.h"
#include "lemon/net/http/http_context.h"
#include "lemon/net/http/http_response.h"
#include "lemon/net/buffer.h"

#include "lemon/base/logger/logger.h"

#include <memory>

using namespace lemon;
using namespace lemon::http;
using namespace lemon::log;

// 默认http回应函数
void defaultHttpCallback(const HttpRequest &, http::HttpResponse *resp)
{
    resp->setStatusCode(http::HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(std::string ip, uint16_t port, const std::string& name, lemon::net::TcpServer::Option option)
: m_address(ip, port)
 , m_mainLoop()
 , m_server(&m_mainLoop, m_address, name, lemon::net::TcpServer::ET, -1, option)
 , m_httpCallback(std::bind(&HttpServer::handleRequest, this, std::placeholders::_1, std::placeholders::_2))
 , m_useSSL(false) {
    init();
}

void HttpServer::start() {
    LOG_INFO("HttpServer[%s] start at %s\n", m_server.name().c_str(), m_server.ipPort().c_str());
    m_server.start();
    m_mainLoop.loop();
}

void HttpServer::init() {
    m_server.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}


void HttpServer::setSslConfig(const ssl::SSLConfig& config) {
    if (m_useSSL) {
        m_sslCtx.reset(new ssl::SSLContext(config));
        if(!m_sslCtx->initialize()) {
            LOG_FATAL("ssl init failed\n");
        }
    }
}

void HttpServer::onConnection(const lemon::net::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        printf("%s -> %s is on\n", conn->peerAddress().getIpPort().c_str()
                            , conn->localAddress().getIpPort().c_str());
        if (m_useSSL) {
            std::unique_ptr<ssl::SSLConnection> sslConn(new ssl::SSLConnection(conn, m_sslCtx.get()));
            sslConn->setMessageCallback(
                std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            m_sslConns[conn] = std::move(sslConn);
            // m_sslConns[conn]->startHandshake(); // FIXME: 第一次读，还没有注册读事件，因此一定握手失败。
        }
        
        conn->setContext(HttpContextKey, std::make_shared<HttpContext>());
    } else {
        // printf("%s -> %s is off\n", conn->peerAddress().getIpPort().c_str()
        //                     , conn->localAddress().getIpPort().c_str());

        if (m_useSSL) {
            m_sslConns.erase(conn);
        }
    }
}

void HttpServer::onMessage(const lemon::net::TcpConnectionPtr& conn,
                lemon::net::Buffer* buf,
                lemon::base::Timestamp receiveTime) {
    try {

        // 如果是ssl，上层会进行解密，然后将解密后的数据放到buf
        // if (m_useSSL) {
        //     LOG_INFO("onMessage useSSL_ is true");
        //     auto it = m_sslConns.find(conn);
        //     if (it != m_sslConns.end()) {
        //         LOG_INFO("onMessage sslConns_ is not empty");
        //         // 2. SSL连接处理数据
        //         // it->second->onRead(conn, buf, receiveTime);// FIXME 应该注释，否则递归？
            
        //         // 3. 如果 SSL 握手还未完成，直接返回
        //         if (!it->second->isHandshakeCompleted()) {
        //             LOG_INFO("onMessage sslConns_ is not empty");
        //             return;
        //         }

        //         // 4. 从SSL连接的解密缓冲区获取数据
        //         lemon::net::Buffer* decryptedBuf = it->second->getDecryptedBuffer();
        //         if (decryptedBuf->readableBytes() == 0)
        //             return; // 没有解密后的数据

        //         // 5. 使用解密后的数据进行HTTP 处理
        //         buf = decryptedBuf; // 将 buf 指向解密后的数据
        //         LOG_INFO("onMessage decryptedBuf is not empty");
        //     }
        // }

        std::shared_ptr<HttpContext> context = conn->getContext<HttpContext>(HttpContextKey);
        if (!context) {
            return;
        }

        if (!context->parseRequest(buf, receiveTime)) {
            LOG_INFO("HttpServer::onMessage() parseRequest error\n");
            http::HttpResponse response(true);
            response.setStatusLine("HTTP/1.1", http::HttpResponse::k400BadRequest, "Bad Request");
            response.setBody("HTTP/1.1 400 Bad Request\r\n\r\n");

            lemon::net::Buffer buffer;
            response.appendToBuffer(&buffer);

            if (m_useSSL) {
                if (m_sslConns.find(conn) != m_sslConns.end()) {
                    m_sslConns[conn]->send(buffer.toString().data(), buffer.readableBytes());
                }
            } else {
                conn->send(buffer.toString());
            }

            conn->shutdown();
            m_sslConns.erase(conn);
            return;
        }

        if (context->gotAll()) {
            onRequest(conn, context->request());
            context->reset();
        }
    } catch( const std::exception& e) {
        LOG_ERROR("HttpServer::onMessage() %s\n", e.what());
        std::string err_msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
        if (m_useSSL) {
            if (m_sslConns.find(conn) != m_sslConns.end()) {
                m_sslConns[conn]->send(err_msg.data(), err_msg.size());
            }
        } else {
            conn->send(err_msg);
        }

        conn->shutdown();
        m_sslConns.erase(conn);
    }         
}
void HttpServer::onRequest(const lemon::net::TcpConnectionPtr& conn, const HttpRequest& req) {
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                (req.version() == "HTTP/1.0" && connection != "Keep-Alive");

    http::HttpResponse response(close);
    m_httpCallback(req, &response);

    lemon::net::Buffer buffer;
    response.appendToBuffer(&buffer);

    // LOG_INFO("HttpServer::onRequest() send response to: %s\n", buffer.toString().c_str());
    if (m_useSSL) {
        if (m_sslConns.find(conn) != m_sslConns.end()) {
            m_sslConns[conn]->send(buffer.toString().data(), buffer.readableBytes());
        }
    } else {
        conn->send(buffer.toString());
    }

    if (response.closeConnection()) {
        conn->shutdown();
        m_sslConns.erase(conn);
    }
}

void HttpServer::handleRequest(const HttpRequest& req, http::HttpResponse* resp) {
    try {
        // 中间件-前
        HttpRequest mutableReq = req;
        m_middlewareChain.processBefore(mutableReq);
        
        // 路由处理
        if (!m_router.route(mutableReq, resp)) {
            LOG_INFO("request url not found. method:%d path:%s\n", mutableReq.method(), req.path().c_str());
            resp->setStatusCode(http::HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }

        // 中间件-后
        m_middlewareChain.processAfter(*resp);

    } catch(const http::HttpResponse& res ) {
        *resp = res;
    } catch(const std::exception& e) {
        LOG_INFO("handleRequest exception: %s\n", e.what());
        resp->setVersion("HTTP/1.1");
        resp->setStatusMessage("Internal Error");
        resp->setStatusCode(http::HttpResponse::k500InternalServerError);
        resp->setCloseConnection(true);
        resp->setBody(e.what());
    }
}

