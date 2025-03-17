#include "lemon/base/logger/async_logging.h"
#include "lemon/base/logger/log_file_appender.h"
#include "lemon/base/timestamp.h"

#include <cstdio>
#include <cassert>

#include <string>

using namespace lemon;
using namespace lemon::log;
using namespace lemon::base;

AsyncLogging::AsyncLogging(const AsyncDoCallback& cb, int flush_interval)
: m_do_callback(std::move(cb))
    , m_flush_interval(flush_interval)
    , m_running(false)
    , m_latch(1) {
    m_thread =  std::unique_ptr<std::thread>(new std::thread(&AsyncLogging::threadWorker, this));
    m_latch.wait();
}

AsyncLogging::~AsyncLogging() {
    if (!m_running) return;
    doDone();
}

void AsyncLogging::doDone() {
    m_running = false;
    m_cond.notify_one();

    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
}

void AsyncLogging::pushMsg(const inner_message& msg) {
    assert(m_running);
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_curBuffer.avail() > 0) {
        m_curBuffer.push(msg);
        return;
    }
    
    // 缓存满了，需要输出到文件
    m_buffers.push_back(std::move(m_curBuffer));    // 转移指针所有权，减少内存拷贝
    if (m_nextBuffer.valid()) {
        // 使用备用内存
        m_curBuffer = std::move(m_nextBuffer);
    } else {
        m_curBuffer = Buffer();
    }
    m_curBuffer.push(msg);
    m_cond.notify_one();
}

void AsyncLogging::threadWorker() {
    try {
        m_running = true;
        // 备用缓存，减少内存分配
        Buffer newBuffer1;
        Buffer newBuffer2;
        
        std::vector<Buffer> buffers_write; // 减小临界区
        buffers_write.reserve(16);
                
        m_latch.countDown();

        while (m_running) {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_buffers.empty()) {
                    m_cond.wait_for(lock, std::chrono::seconds(m_flush_interval));
                }

                // FIXME: 为什么m_curBuffer还有空间，要着急处理？
                // 因为m_buffers此时可能有数据, 需要先将m_curBuffer输出
                if (m_buffers.empty() && m_curBuffer.hasData()) {
                    m_buffers.push_back(std::move(m_curBuffer));
                    m_curBuffer = std::move(newBuffer1);
                }

                buffers_write.swap(m_buffers);
                if (!m_nextBuffer.valid()) {
                    m_nextBuffer = std::move(newBuffer2);
                }
            }

            if (buffers_write.empty()) {
                continue;
            }

            // FIXME: buffert太大，一次性要移除这么多？
            if (buffers_write.size() > 100) {
                char buf[256];
                snprintf(buf, sizeof(buf),
                        "Dropped log messages at %s, %zd larger buffers\n",
                        Timestamp::now().toString().c_str(), buffers_write.size() - 2);
                fputs(buf, stderr);
                m_do_callback(buf);
                buffers_write.erase(buffers_write.begin() + 2, buffers_write.end());
            }
            
            for (const auto& buffer : buffers_write) {
                for (const auto& it : buffer) {
                    m_do_callback(it.msg.c_str());
                }
            }

            if (buffers_write.size() < 2) {
                buffers_write.resize(2);
            }

            if (!newBuffer1.valid()) {
                newBuffer1 = std::move(buffers_write.back());
                buffers_write.pop_back();
                newBuffer1.reset();
            }

            if (!newBuffer2.valid()) {
                newBuffer2 = std::move(buffers_write.back());
                buffers_write.pop_back();
                newBuffer2.reset();
            }

            buffers_write.clear(); 
            if (m_flush_callback) {
                m_flush_callback();
            }
        }
    } catch (const std::exception& e) {
        fprintf(stderr, "Error log thread, err=%s\n", e.what());
        m_thread.reset();
    }
}