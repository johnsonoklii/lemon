#ifndef LEMON_POLLER_H
#define LEMON_POLLER_H

#include "lemon/base/nocopyable.h"
#include "lemon/base/timestamp.h"
#include "lemon/net/eventloop.h"

#include <vector>
#include <map>

namespace lemon {
namespace net {
using namespace base;

class Channel;

class Poller: noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeout_ms, ChannelList* active_channels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    virtual bool hasChannel(Channel* channel) const;
    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread() const {
        m_loop->assertInLoopThread();
    }

protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap m_channels;

private:
    EventLoop* m_loop;
}
;

} // namespace net
} // namespace lemon

#endif