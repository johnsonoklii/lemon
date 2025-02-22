#ifndef LEMON_EVENTLOOP_THREAD_H
#define LEMON_EVENTLOOP_THREAD_H

#include "lemon/net/eventloop.h"
#include "lemon/base/thread.h"

#include <memory>
#include <mutex>
#include <condition_variable>

namespace lemon {
namespace net {

class EventLoopThread: noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* m_loop;
    Thread m_thread;
    bool m_exiting;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    ThreadInitCallback m_callback;
};

} // namespace net
} // namespace lemon

#endif