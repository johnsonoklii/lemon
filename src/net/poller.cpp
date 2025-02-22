#include "lemon/net/poller.h"
#include "lemon/net/channel.h"

using namespace lemon;
using namespace lemon::net;

Poller::Poller(EventLoop* loop)
    : m_loop(loop) {
}

bool Poller::hasChannel(Channel* channel) const {
    assertInLoopThread();
    ChannelMap::const_iterator it = m_channels.find(channel->fd());
    return it != m_channels.end() && it->second == channel;
}