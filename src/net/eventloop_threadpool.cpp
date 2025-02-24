#include "lemon/net/eventloop_threadpool.h"
#include "lemon/net/eventloop.h"
#include "lemon/net/eventloop_thread.h"

#include "lemon/base/logger/logger.h"

#include <assert.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const std::string& name)
: m_base_loop(base_loop)
 , m_name(name)
 , m_started(false)
 , m_num_threads(0)
 , m_next(0) {

}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::start(const ThraedInitCallback& cb) {
    assert(!m_started);
    m_base_loop->assertInLoopThread();
    
    m_started = true;
    for (int i = 0; i < m_num_threads; ++i) {
        char buf[m_name.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", m_name.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
        m_loops.push_back(t->startLoop());
    }

    if (m_num_threads == 0 && cb) {
        cb(m_base_loop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    m_base_loop->assertInLoopThread();
    assert(m_started);

    EventLoop* loop = m_base_loop;

    if (!m_loops.empty()) {
        loop = m_loops[m_next];
        ++m_next;
        if (static_cast<size_t>(m_next) >= m_loops.size()) {
            m_next = 0;
        }
    }

    return loop;
}
EventLoop* EventLoopThreadPool::getLoopForHash(size_t hash_code) {
    m_base_loop->assertInLoopThread();
    assert(m_started);

    EventLoop* loop = m_base_loop;
    if (!m_loops.empty()) {
        loop = m_loops[hash_code % m_loops.size()];
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    m_base_loop->assertInLoopThread();
    assert(m_started);
    if (!m_loops.empty()) {
        return m_loops;
    } else {
        return std::vector<EventLoop*>(1, m_base_loop);
    }
}