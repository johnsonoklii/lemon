#include "lemon/net/eventloop.h"
#include "lemon/net/channel.h"
#include "lemon/base/logger/logger.h"
#include "lemon/net/poller.h"

#include <algorithm>
#include <sys/eventfd.h>
#include <cassert>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

thread_local EventLoop* t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10000;

int createEventFd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("Failed in eventfd\n");
    }
    return evtfd;
}

EventLoop::EventLoop()
: m_looping(false)
 , m_event_handling(false)
 , m_iteration(0)
 , m_thread_id(ProcessInfo::tid())
 , m_poller(Poller::newDefaultPoller(this))
 , m_wakeup_fd(createEventFd())
 , m_wakeup_channel(new Channel(this, m_wakeup_fd))
 , m_current_active_channel(nullptr)
 , m_mutex()
 , m_pending_functors(){
    LOG_DEBUG("EventLoop created %p in thread %d\n", this, m_thread_id);
    if (t_loop_in_this_thread) {
        LOG_FATAL("Another EventLoop %p exists in this thread %d\n", t_loop_in_this_thread, m_thread_id);
    } else {
        t_loop_in_this_thread = this;
    }

    m_wakeup_channel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    m_wakeup_channel->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG("EventLoop %p of thread %d destructs\n", this, m_thread_id);
    m_wakeup_channel->disableAll();
    m_wakeup_channel->remove();
    ::close(m_wakeup_fd);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::loop() {
    assert(!m_looping);
    assertInLoopThread();
    m_looping = true;
    
    LOG_DEBUG("EventLoop %p start looping\n", this);

    // epoll_wait()
    while (m_looping) {
        m_active_channels.clear();
        m_poll_return_time = m_poller->poll(kPollTimeMs, &m_active_channels);
        ++m_iteration;

        m_event_handling = true;
        for (Channel* channel : m_active_channels) {
            m_current_active_channel = channel;
            m_current_active_channel->handleEvent(m_poll_return_time);
        }

        m_current_active_channel = nullptr;
        m_event_handling = false;
        doPendingFunctors();
    }
}

void EventLoop::stop() {
    m_looping = false;
    if (!isInLoopThread()) {
        wakeup();
    }
}

size_t EventLoop::queueSize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pending_functors.size();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(m_wakeup_fd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8\n", n);
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(m_wakeup_fd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8\n", n);
    }
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (m_event_handling) {
        assert(m_current_active_channel == channel || 
                std::find(m_active_channels.begin(), m_active_channels.end(), channel) == m_active_channels.end());
        m_current_active_channel = nullptr;
    }
    m_poller->removeChannel(channel);
}

void EventLoop::runInLoop(const Functor& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}
    
void EventLoop::queueInLoop(const Functor& cb) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending_functors.push_back(cb);
    }

    if (!isInLoopThread() || m_calling_pending_funcs) {
        wakeup();
    }
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return m_poller->hasChannel(channel);
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loop_in_this_thread;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL("EventLoop::abortNotInLoopThread - EventLoop %p was created in threadId_ = %d, current thread id = %d\n",
              this, m_thread_id, ProcessInfo::tid());
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    m_calling_pending_funcs = true;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(m_pending_functors);
    }

    for (const Functor& functor : functors) {
        functor();
    }

    m_calling_pending_funcs = false;
}
