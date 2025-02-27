#ifndef LEMON_TIME_SET_H
#define LEMON_TIME_SET_H

#include "lemon/base/nocopyable.h"
#include "lemon/net/timer.h"
#include "lemon/net/timerid.h"
#include "lemon/net/channel.h"
#include "lemon/net/callbacks.h"

#include <set>
#include <vector>   

namespace lemon {
namespace net {

using namespace base;

class EventLoop;

class TimerSet: noncopyable {
public:
    using Entry = std::pair<Timestamp, std::shared_ptr<Timer>>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<std::shared_ptr<Timer>, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    explicit TimerSet(EventLoop* loop);
    ~TimerSet();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerId timer_id);

private:
    void addTimerInLoop(std::shared_ptr<Timer> timer);
    void cancelInLoop(TimerId timer_id);
    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(std::shared_ptr<Timer> timer);

    EventLoop* m_loop;
    const int m_timerfd;
    Channel m_timerfdChannel;
    TimerList m_timers;

    ActiveTimerSet m_activeTimers;
    bool m_callingExpiredTimers;
    ActiveTimerSet m_cancelingTimers;

    std::vector<Entry> m_expiredList;
};

} // namespace net
} // namespace lemon


#endif