#include "lemon/net/http/http_server.h"
#include "lemon/net/http/middleware/cors/cors_middleware.h"
#include "lemon/net/http/session/session_storage.h"

#include "lemon/net/http/ssl/ssl_config.h"
#include "lemon/base/logger/logger.h"

using namespace lemon;
using namespace lemon::net;
using namespace lemon::http;
using namespace lemon::log;

#include <memory>

int main() {

    HttpServer server("0.0.0.0", 443, "https_test");


    // 加载 SSL 配置
    ssl::SSLConfig sslConfig;
    // 获取当前工作目录
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        LOG_INFO("Current working directory: %s", cwd);
    }

    // 设置证书文件（使用绝对路径）
    std::string certFile = "/etc/letsencrypt/live/growingshark.asia/fullchain.pem";
    std::string keyFile = "/etc/letsencrypt/live/growingshark.asia/privkey.pem";
    LOG_INFO("Loading certificate from: %s", certFile);
    LOG_INFO("Loading private key from: %s", keyFile);

    sslConfig.setCertificateFile(certFile);
    sslConfig.setPrivateKeyFile(keyFile);
    sslConfig.setProtocolVersion(ssl::SSLVersion::TLS_1_2);

    // 验证文件是否存在
    if (access(certFile.c_str(), R_OK) != 0) {
        LOG_FATAL("Cannot read certificate file:  %s", certFile);
    }

    if (access(keyFile.c_str(), R_OK) != 0) {
        LOG_FATAL("Cannot read private key file:  %s", keyFile);

    }

    server.enableSSL(true);
    server.setSslConfig(sslConfig);

    server.Get("/", [&](const HttpRequest& req, HttpResponse* resp) {
        // (void)req;
        std::string body = "default response";
        std::shared_ptr<session::Session> session = server.sessionManager()->getSession(req, resp);
        if (session->getValue("isLoggedIn") != "true") {
            // 需要登录
            session->setValue("isLoggedIn", "true");
            session->setValue("username", "johnsonoklii");

            body = "<html><head><title>This is title</title></head><body>Login Success! This is a html response</body></html>";
        } else {
            std::string username =  session->getValue("username");;
            char buf[1024];
            snprintf(buf, sizeof(buf), "<html><head><title>This is title</title></head><body> welcome %s! </body></html>", username.c_str());
            body = buf;
        }

        resp->setVersion("HTTP/1.1");
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setCloseConnection(true);
        resp->setContentType("text/html");
        resp->setBody(body);
       
    });

    server.start();

    return 0;
}