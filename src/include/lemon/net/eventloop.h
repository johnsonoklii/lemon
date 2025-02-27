#ifndef LEMON_EVENTLOOP_H
#define LEMON_EVENTLOOP_H 

#include "lemon/base/nocopyable.h"
#include "lemon/base/timestamp.h"
#include "lemon/base/utils.h"
#include "lemon/net/timerid.h"
#include "lemon/net/callbacks.h"

#include <unistd.h>

#include <memory>
#include <vector>
#include <mutex>
#include <functional>  

namespace lemon {
namespace net {

using namespace base;

class Channel;
class Poller;
class TimerSet;

class EventLoop: public noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();

    void stop();

    Timestamp pollReturnTime() const { return m_poll_return_time; }
    
    void runInLoop(const Functor& cb);
    
    void queueInLoop(const Functor& cb);

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

    // timers
    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);

    static EventLoop* getEventLoopOfCurrentThread();

private:
    using ChannelList = std::vector<Channel*>;

    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    bool m_looping; // FIXME: change to Atomic

    bool m_event_handling;
    bool m_calling_pending_funcs;
    
    int64_t m_iteration;
    const pid_t m_thread_id;
    Timestamp m_poll_return_time;

    std::unique_ptr<Poller> m_poller;
    std::unique_ptr<TimerSet> m_timeSet;
    
    int m_wakeup_fd;    // 其他线程使用loop添加了待执行的函数，通过wakeup_fd唤醒loop的poller
    std::unique_ptr<Channel> m_wakeup_channel;

    ChannelList m_active_channels;
    Channel* m_current_active_channel;

    std::mutex m_mutex;
    std::vector<Functor> m_pending_functors;
};

} // namespace net
} // namespace lemon

#endif