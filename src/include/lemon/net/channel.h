#ifndef LEMON_CHANNEL_H
#define LEMON_CHANNEL_H

#include "lemon/base/nocopyable.h"
#include "lemon/base/timestamp.h"

#include <functional>
#include <memory>
#include <sys/poll.h>
// #include <sys/epoll.h>

namespace lemon {
namespace net {

using namespace base;

class EventLoop;

class Channel: public noncopyable{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp recv_time);

    void tie(const std::shared_ptr<void>&);
    int fd() const { return m_fd; }
    int events() const { return m_events; }

    void setTriMode(int triMode) { m_triMode = triMode; }

    void setRevents(int revt) { m_revents = revt; }
    bool isNoneEvent() const { return m_events == kNoneEvent; }
    
    void enableReading() { m_events |= (kReadEvent | m_triMode); update(); }
    void disableReading() { m_events &= ~kReadEvent; update(); }
    void enableWriting() { m_events |= (kWriteEvent | m_triMode); update(); }
    void disableWriting() { m_events &= ~kWriteEvent; update(); }
    void disableAll() { m_events = kNoneEvent; update(); }
    bool isWriting() const { return m_events & kWriteEvent; }
    bool isReading() const { return m_events & kReadEvent; }

    int index() { return m_index; }
    void setIndex(int idx) { m_index = idx; }

    void remove();

    void setReadCallback(const ReadEventCallback& cb) {
        m_read_callback = cb;
    }

    void setWriteCallback(const EventCallback& cb) {
        m_write_callback = cb;
    }

    void setCloseCallback(const EventCallback& cb) {
        m_close_callback = cb;
    }

    void setErrorCallback(const EventCallback& cb) {
        m_error_callback = cb;
    }

    EventLoop* ownerLoop() { return m_loop; }

private:
    void update();
    void handleEventWithGuard(Timestamp recv_time);

    enum {
        kNoneEvent = 0,
        kReadEvent =  POLLIN | POLLPRI,
        kWriteEvent = POLLOUT
    };

    EventLoop* m_loop;

    int m_fd;
    int m_events;
    int m_revents;  // 触发的事件
    int m_index;
    bool m_logHup;
    int m_triMode;

    std::weak_ptr<void> m_tie;
    bool m_tied;
    bool m_added_to_loop;
    bool m_event_handling;

    ReadEventCallback m_read_callback;
    EventCallback m_write_callback;
    EventCallback m_close_callback;
    EventCallback m_error_callback;
};

} // namespace net
} // namespace lemon



#endif
