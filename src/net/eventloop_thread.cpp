#include "lemon/net/eventloop_thread.h"
#include "lemon/base/logger/logger.h"

#include <assert.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                    const std::string& name)
: m_loop(nullptr)
 , m_thread(std::bind(&EventLoopThread::threadFunc, this), name)
 , m_exiting(false)
 , m_mutex()
 , m_cond()
 , m_callback(cb) {

}

EventLoopThread::~EventLoopThread() {
    m_exiting = true;
    if (m_loop != nullptr) {
        m_loop->stop();
        m_thread.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!m_thread.started());
    m_thread.start();

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_loop == nullptr) {
            m_cond.wait(lock);
        }
    }

    loop = m_loop;
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    if (m_callback) {
        m_callback(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_one();
    }

    loop.loop();

    // FIXME: 为什么要加锁？防止其他线程使用m_loop，
    // 如果线程A 正在if(m_loop)判断成功后，此时该线程m_loop=nullptr，就会出现问题
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}