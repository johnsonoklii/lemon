#ifndef LEMON_EPOLLPOLLER_H
#define LEMON_EPOLLPOLLER_H

#include "lemon/net/poller.h"

struct epoll_event;

namespace lemon {
namespace net {

class EpollPoller: public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp poll(int timeout_ms, ChannelList* active_channels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;
    
    void fillActiveChannels(int num_events, ChannelList* active_channels) const;
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;

    int m_epollfd;
    EventList m_events;
};

} // namespace net
} // namespace lemon

#endif 