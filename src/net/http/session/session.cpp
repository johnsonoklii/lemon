#include "lemon/net/http/session/session.h"

using namespace lemon::http::session;

Session::Session(const std::string& sessionId, SessionManager* manager, int maxAge)
: m_sessionId(sessionId)
 , m_max_age(maxAge)
 , m_manager(manager) {
    refresh();
}

void Session::setValue(const std::string& key, const std::string& value) {
    m_data[key] = value;
    if (m_manager) {
        m_manager->updateSession(shared_from_this()); // 必须要public继承enable_shared_from_this，否则报错 bad_weak_ptr
    }
}

std::string Session::getValue(const std::string& key) const {
    return m_data.count(key) ? m_data.at(key) : "";
}

void Session::remove(const std::string& key) {
    m_data.erase(key);
}

void Session::clear() {
    m_data.clear();
}

bool Session::isExpired() const {
    return std::chrono::system_clock::now() > m_expire_time;
}

void Session::refresh() {
    m_expire_time = std::chrono::system_clock::now() + std::chrono::seconds(m_max_age);
}