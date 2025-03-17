#ifndef LEMON_HTTP_SESSION_MANAGER_H_
#define LEMON_HTTP_SESSION_MANAGER_H_

#include "lemon/net/http/http_request.h"
#include "lemon/net/http/http_response.h"

// #include "lemon/net/http/session/session.h"
// #include "lemon/net/http/session/session_storage.h"

#include <memory>
#include <random>

namespace lemon {
namespace http {
namespace session {

class SessionStorage;
class Session;

class SessionManager {
public:
    explicit SessionManager(SessionStorage* storage);
    
    std::shared_ptr<Session> getSession(const HttpRequest& req, HttpResponse* resp);
    void destroySession(const std::string& sessionId);
    void clearExpiredSessions();
    void updateSession(std::shared_ptr<Session> session);

private:
    std::string generateSessionId();
    std::string getSessionIdFromCookie(const HttpRequest& request);
    void setSessionCookie(HttpResponse* response, const std::string& sessionId);
private:
    SessionStorage* m_storage;
    std::mt19937 m_rng; // 用于生成随机会话id
};

} // namesapce session
} // namespace http
} // namespace lemon

#endif