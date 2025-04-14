#include "lemon/net/http/session/session_manager.h"
#include "lemon/net/http/session/session.h"
#include "lemon/net/http/session/session_storage.h"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace lemon::http::session;
SessionManager::SessionManager(SessionStorage* storage)
: m_storage(storage)
 , m_rng(std::random_device{}()) {

}
    
std::shared_ptr<Session> SessionManager::getSession(const HttpRequest& req, HttpResponse* resp) {
    std::string sessionId = getSessionIdFromCookie(req);

    std::shared_ptr<Session> session;
    
    if (!sessionId.empty()) {
        session = m_storage->load(sessionId);
    }

    // session == null : 1-sessionId.empty()==null or 2-m_storage->load == null
    // 没有session，或者session已经过期，需要创建新的session
    if (!session || session->isExpired()) {
        sessionId = generateSessionId();
        session = std::make_shared<Session>(sessionId, this);
        setSessionCookie(resp, sessionId);
        m_storage->save(session);
    } else {
        session->setManager(this);
    }

    session->refresh();
    
    return session;

    // 存在重复代码
    // std::string sessionId = getSessionIdFromCookie(req);
    // std::shared_ptr<Session> session;
    // if (sessionId.empty()) {
    //     // 创建一个新session
    //     sessionId = generateSessionId();
    //     session = std::make_shared<Session>(sessionId, this);
    //     m_storage->save(session);
    //     setSessionCookie(resp, sessionId);
    // } else {
    //     session = m_storage->load(sessionId);
    //     if (!session || session->isExpired()) {
    //         // session已经过期，创建一个新的session
    //         sessionId = generateSessionId();
    //         session = std::make_shared<Session>(sessionId, this);
    //         m_storage->save(session);
    //         setSessionCookie(resp, sessionId);
    //     } else {
    //         session->setManager(this);
    //     }
    // }
    // return session;
}

void SessionManager::destroySession(const std::string& sessionId) {
    m_storage->remove(sessionId);
}

void SessionManager::clearExpiredSessions() {
    m_storage->clearExpiredSessions();
}

void SessionManager::updateSession(std::shared_ptr<Session> session) {
    m_storage->save(session);
}

std::string SessionManager::generateSessionId() {
    std::stringstream ss;
    std::uniform_int_distribution<> dist(0, 15);

    // 生成32个字符的会话ID，每个字符是一个十六进制数字
    for (int i = 0; i < 32; ++i)
    {
        ss << std::hex << dist(m_rng);
    }
    return ss.str();
}

std::string SessionManager::getSessionIdFromCookie(const HttpRequest& request) {
    std::string sessionId;
    std::string cookie = request.getHeader("Cookie");
    if (!cookie.empty()) {
        size_t pos = cookie.find("sessionId=");
        if (pos != std::string::npos) {
            pos += 10; // 跳过"sessionId="
            size_t end = cookie.find(';', pos);
            if (end != std::string::npos) {
                sessionId = cookie.substr(pos, end - pos);
            }
            else {
                sessionId = cookie.substr(pos);
            }
        }
    }

    return sessionId;
}   

void SessionManager::setSessionCookie(HttpResponse* response, const std::string& sessionId) {
    // 设置会话ID到响应头中，作为Cookie
    std::string cookie = "sessionId=" + sessionId + "; Path=/; HttpOnly";
    response->addHeader("Set-Cookie", cookie);
}