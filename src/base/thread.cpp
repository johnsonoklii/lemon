#include "lemon/base/thread.h"
#include "lemon/base/utils.h"

#include <cassert>

using namespace lemon;
using namespace lemon::base;

std::atomic_int Thread::m_numCreated;

Thread::Thread(ThreadFunc func, const std::string& name)
: m_started(false)
 , m_joined(false)
 , m_tid(0)
 , m_func(std::move(func))
 , m_name(name)
 , m_latch(1) {
    setDefaultName();
}

Thread::~Thread(){
  if (m_started && !m_joined) {
    m_thread.detach();
  }
}

void Thread::setDefaultName() {
    int num = m_numCreated.fetch_add(1);
    if (m_name.empty()) {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        m_name = buf;
    }
}

void Thread::runInThread() {
    m_tid = ProcessInfo::tid();
    m_latch.countDown();
    m_func();
}

void Thread::start() {
    assert(!m_started);
    m_started = true;
    m_thread = std::thread(std::bind(&Thread::runInThread, this));

    m_latch.wait();
}

void Thread::join() {
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    m_thread.join();
}