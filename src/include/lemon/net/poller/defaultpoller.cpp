#include "lemon/net/poller.h"
#include "lemon/net/poller/epollpoller.h"

using namespace lemon::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  return new EpollPoller(loop);
}
