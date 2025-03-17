#ifndef LEMON_HTTP_SESSION_H
#define LEMON_HTTP_SESSION_H

#include "lemon/net/http/session/session_manager.h"

#include <unordered_map>
#include <string>
#include <chrono>

namespace lemon {
namespace http {
namespace session {

class Session: public std::enable_shared_from_this<Session> {
public:
    Session(const std::string& sessionId, SessionManager* manager, int maxAge = 3600);

    void setValue(const std::string& key, const std::string& value);
    std::string getValue(const std::string& key) const;
    void remove(const std::string& key);
    void clear();

    bool isExpired() const;
    // 刷新过期时间
    void refresh();

    SessionManager* getManager() const { 
        return m_manager; 
    }
    void setManager(SessionManager* manager) {
        m_manager = manager;
    }
    
    const std::string& getId() const { return m_sessionId; }

private:
    std::string m_sessionId;
    std::unordered_map<std::string, std::string> m_data;
    std::chrono::system_clock::time_point m_expire_time; // 最大过期时间(秒)
    int m_max_age;

    SessionManager* m_manager;
};

} // namespace session
} // namespace http
} // namespace lemon

#endif