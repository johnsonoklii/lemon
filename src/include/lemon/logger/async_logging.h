#ifndef LEMON_ASYNC_LOGGING_H
#define LEMON_ASYNC_LOGGING_H

#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>   
#include <vector>
#include <functional>

#include "lemon/logger/log_buffer.h"
#include "lemon/base/count_down_latch.h"

namespace lemon {
namespace log {

class AsyncLogging {
public:
    using AsyncDoCallback = std::function<void(const char*)>;
    using FlushCallback = std::function<void()>;
    AsyncLogging() = default;
    AsyncLogging(const AsyncDoCallback& cb, int flush_interval);
    ~AsyncLogging();

    void pushMsg(const inner_message& msg);

    void doDone();

    void doCallback(const char* msg);

    void setDoCallback(const AsyncDoCallback& cb) {
        m_do_callback = std::move(cb);
    }

    void setFlushCallback(const FlushCallback& cb) {
        m_flush_callback = std::move(cb);
    }

private:
    using Buffer = LogBuffer<kLargeBuffer>;
    void threadWorker();
    AsyncDoCallback m_do_callback;
    FlushCallback m_flush_callback;

    const int m_flush_interval;
    std::atomic_bool m_running;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::unique_ptr<std::thread> m_thread;

    base::CountDownLatch m_latch;

    Buffer m_curBuffer;    // 当前缓冲区
    Buffer m_nextBuffer;   // 预留，减少内存分配
    
    std::vector<Buffer> m_buffers;
};

} // namespace log
} // namespace lemon

#endif