#ifndef LEMON_EVENTLOOP_H
#define LEMON_EVENTLOOP_H 

#include "lemon/base/nocopyable.h"
#include "lemon/base/timestamp.h"
#include "lemon/base/utils.h"
#include "lemon/base/co/fiber.h"

#include <unistd.h>

#include <memory>
#include <vector>
#include <mutex>
#include <functional>  

namespace lemon {
namespace net {

using namespace base;
using namespace co;

class Channel;
class Poller;

class EventLoop: public noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();  // 协程实现
    void loop2(); // muduo库中的实现

    void stop();

    Timestamp pollReturnTime() const { return m_poll_return_time; }
    
    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);

    void queueInLoopFiber(const Functor& cb);
    void addTask(const Fiber::FunCallback& cb);

    size_t queueSize();

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const {
        return m_thread_id == ProcessInfo::tid();
    }

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool eventHandling() const {
        return m_event_handling;
    }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    using ChannelList = std::vector<Channel*>;

    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    void doFiberTasks();

    bool m_looping; // FIXME: change to Atomic

    bool m_event_handling;
    bool m_calling_pending_funcs;
    
    int64_t m_iteration;
    const pid_t m_thread_id;
    Timestamp m_poll_return_time;

    std::unique_ptr<Poller> m_poller;
    
    int m_wakeup_fd;    // 其他线程使用loop添加了待执行的函数，通过wakeup_fd唤醒loop的poller
    std::unique_ptr<Channel> m_wakeup_channel;

    ChannelList m_active_channels;
    Channel* m_current_active_channel;

    std::mutex m_mutex;
    std::vector<Functor> m_pending_functors;

    std::vector<Fiber::Ptr> m_fiber_tasks;
};

} // namespace net
} // namespace lemon

#endif