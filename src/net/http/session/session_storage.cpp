#include "lemon/net/http/session/session_storage.h"
#include "lemon/net/http/session/session.h"

using namespace lemon::http::session;

void MemorySessionStorage::save(std::shared_ptr<Session> session) {
    m_sessions[session->getId()] = session;
}

std::shared_ptr<Session>  MemorySessionStorage::load(const std::string& sessionId) {
    auto it = m_sessions.find(sessionId);
    if (it != m_sessions.end()) {
        if (!it->second->isExpired()) {
            return it->second;
        } else {
            // 如果会话已过期，则从存储中移除
            m_sessions.erase(it);
        }
    }

    // 如果会话不存在或已过期，则返回nullptr
    return nullptr;
}

void MemorySessionStorage::remove(const std::string& sessionId) {
    m_sessions.erase(sessionId);
}

void MemorySessionStorage::clearExpiredSessions() {

}