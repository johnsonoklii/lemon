#ifndef LEMON_EVENTLOOP_THREADPOOL_H
#define LEMON_EVENTLOOP_THREADPOOL_H

#include "lemon/base/nocopyable.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace lemon {
namespace net {

using namespace base;

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool: noncopyable{
public:
    using ThraedInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* base_loop, const std::string& name);
    ~EventLoopThreadPool();

    void setThreadNum(int num_threads) { m_num_threads = num_threads; }
    void start(const ThraedInitCallback&cb = ThraedInitCallback());
    
    EventLoop* getNextLoop();
    EventLoop* getLoopForHash(size_t hash_code);
    std::vector<EventLoop*> getAllLoops();
private:
    EventLoop* m_base_loop;
    std::string m_name;
    bool m_started;
    int m_num_threads;
    int m_next;
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop*> m_loops;
};

} // namespace net
} // namespace lemon

#endif