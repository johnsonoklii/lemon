#ifndef LEMON_THREAD_H
#define LEMON_THREAD_H

#include "lemon/base/nocopyable.h"
#include "lemon/base/count_down_latch.h"

#include <string>
#include <functional>
#include <atomic>
#include <thread>

#include <pthread.h>

namespace lemon {
namespace base {

class Thread: noncopyable {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string& name = std::string());

    ~Thread();

    void start();
    void join();

    bool started() const { return m_started; }
    pid_t tid() const { return m_tid; }
    const std::string& name() const { return m_name; }

private:
    void setDefaultName();
    void runInThread();

    bool m_started;
    bool m_joined;
    pid_t m_tid;
    ThreadFunc m_func;
    std::string m_name;
    CountDownLatch m_latch;
    std::thread m_thread;
    static std::atomic_int m_numCreated;
};

} // namespace base
} // namespace lemon

#endif