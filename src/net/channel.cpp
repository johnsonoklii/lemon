#include "lemon/net/channel.h"
#include "lemon/net/eventloop.h"
#include "lemon/logger/logger.h"

#include <assert.h>
#include <sys/poll.h>

using namespace lemon::net;
using namespace lemon::log;

Channel::Channel(EventLoop* loop, int fd)
: m_loop(loop)
 , m_fd(fd)
 , m_events(0)
 , m_revents(0)
 , m_index(-1)
 , m_logHup(false)
 , m_triMode(0)
 , m_tie()
 , m_tied(false)
 , m_added_to_loop(false)
 , m_event_handling(false)
 , m_read_callback(nullptr)
 , m_write_callback(nullptr)
 , m_close_callback(nullptr)
 , m_error_callback(nullptr) {

}

Channel::~Channel() {
    assert(!m_event_handling);
    assert(!m_added_to_loop);

    if (m_loop->isInLoopThread()) {
        assert(!m_loop->hasChannel(this));
    }
}

void Channel::handleEvent(Timestamp recv_time) {
    std::shared_ptr<void> guard;
    if (m_tied) {
        guard = m_tie.lock();
        if (guard) {
            handleEventWithGuard(recv_time);
        }
    } else {
        handleEventWithGuard(recv_time);
    }
}

void Channel::handleEventWithGuard(Timestamp recv_time) {
    m_event_handling = true;
    // 对端关闭，并且没有可读消息
    if ((m_revents & POLLHUP) && !(m_revents & POLLIN)) {
        if (m_logHup) {
            LOG_WARN("fd = %d, Channel::handle_event() POLLHUP\n", m_fd);
        }
        if (m_close_callback) {
            LOG_INFO("Channel::handleEventWithGuard: peer side closed....\n");
            m_close_callback();
        }
    }

    // 无效请求
    if (m_revents & POLLNVAL) {
        LOG_WARN("fd = %d, Channel::handle_event() POLLNVAL\n", m_fd);
    }

    if (m_revents & (POLLERR|POLLNVAL)) {
        if (m_error_callback) {
            m_error_callback();
        }
    }

    // POLLPRI: 紧急消息; POLLRDHUP: 对端关闭
    if (m_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (m_read_callback) {
            m_read_callback(recv_time);
        }
    }

    if (m_revents & POLLOUT) {
        if (m_write_callback) {
            m_write_callback();
        }
    }

    m_event_handling = false;
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    m_tie = obj;
    m_tied = true;
}

void Channel::remove() {
    assert(isNoneEvent());
    m_added_to_loop = false;
    m_loop->removeChannel(this);
}

void Channel::update() {
    m_added_to_loop = true;
    m_loop->updateChannel(this);
}
