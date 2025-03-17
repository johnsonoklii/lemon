#include "lemon/net/http/http_server.h"
#include "lemon/net/http/middleware/cors/cors_middleware.h"
#include "lemon/net/http/session/session_storage.h"

using namespace lemon;
using namespace lemon::net;
using namespace lemon::http;

#include <memory>

int main() {

    HttpServer server("0.0.0.0", 7777, "http_test");

    server.addMiddleware(std::make_shared<http::middleware::CorsMiddleware>());

    session::MemorySessionStorage storage;
    session::SessionManager manager(&storage);
    server.setSessionManager(&manager);


    server.setThreadNum(3);

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