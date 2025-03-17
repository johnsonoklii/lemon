#ifndef LEMON_HTTP_SESSION_STORAGE_H
#define LEMON_HTTP_SESSION_STORAGE_H

// #include "lemon/net/http/session/session.h"

#include <unordered_map>
#include <memory>
#include <string>

namespace lemon {
namespace http {
namespace session {

class Session;

class SessionStorage {
public:
    virtual ~SessionStorage() = default;

    virtual std::shared_ptr<Session> load(const std::string& sessionId) = 0;
    virtual void save(std::shared_ptr<Session> session) = 0;
    virtual void remove(const std::string& sessionId) = 0;

    virtual void clearExpiredSessions() = 0;
};

// 基于内存的会话存储实现
class MemorySessionStorage : public SessionStorage
{
public:
    void save(std::shared_ptr<Session> session) override;
    std::shared_ptr<Session> load(const std::string& sessionId) override;
    void remove(const std::string& sessionId) override;

    void clearExpiredSessions() override;
private:
    std::unordered_map<std::string, std::shared_ptr<Session>> m_sessions;
};


} // namespace session
} // namespace http
} // namespace lemon

#endif