#include "lemon/net/poller/epollpoller.h"
#include "lemon/net/channel.h"
#include "lemon/logger/logger.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

namespace {
    // channel对于epoll_fd的状态
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

EpollPoller::EpollPoller(EventLoop* loop)
: Poller(loop)
 , m_epollfd(::epoll_create1(EPOLL_CLOEXEC)) 
 , m_events(kInitEventListSize) {
    if (m_epollfd < 0) {
        LOG_FATAL("epoll_create error: %d\n", errno);
    }
}

EpollPoller::~EpollPoller() {
    ::close(m_epollfd);
}

Timestamp EpollPoller::poll(int timeout_ms, ChannelList* active_channels) {
    int num_events = ::epoll_wait(m_epollfd, &*m_events.begin(), static_cast<int>(m_events.size()), timeout_ms);
    int saved_errno = errno;
    Timestamp now = Timestamp::now();

    if (num_events > 0) {
        fillActiveChannels(num_events, active_channels);
        if (static_cast<size_t>(num_events) == m_events.size()) {
            m_events.resize(m_events.size() * 2);
        }
    } else if (num_events == 0) {
        // LOG_WARN("nothing happened\n");
    } else {
        if (saved_errno != EINTR) {
            errno = saved_errno;
            LOG_ERROR("EpollPoller::poll()\n");
        }
    }

    return now;
}

void EpollPoller::fillActiveChannels(int num_events, ChannelList* active_channels) const {
    for (int i = 0; i < num_events; ++i) {
        Channel* channel = static_cast<Channel*>(m_events[i].data.ptr);
        
#ifdef LEMON_DEBUG 
        int fd = channel->fd();
        ChannelMap::const_iterator it = m_channels.find(fd);
        assert(it != m_channels.end());
        assert(it->second == channel);
#endif
        channel->setRevents(m_events[i].events);
        active_channels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();

    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            assert(m_channels.find(fd) == m_channels.end());
            m_channels[fd] = channel;
        } else { // index == kDeleted
            assert(m_channels.find(fd) != m_channels.end());
            assert(m_channels[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        assert(m_channels.find(fd) != m_channels.end());
        assert(m_channels[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    assert(m_channels.find(fd) != m_channels.end());
    assert(m_channels[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = m_channels.erase(fd);
    assert(n == 1);

    if (index == kAdded)
    {
      update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EpollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        } else {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}
