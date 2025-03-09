#include "lemon/net/timer_set.h"
#include "lemon/net/eventloop.h"
#include "lemon/net/timerid.h"

#include "lemon/base/timestamp.h"
#include "lemon/base/logger/logger.h"

#include <sys/timerfd.h>
#include <string.h>
#include <assert.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

int createTimerFd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL("TimerSet::createTimerFd\n");
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch()
                            - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    // newValue.it_interval
    
    if (0 != ::timerfd_settime(timerfd, 0, &newValue, &oldValue)) {
        LOG_ERROR("TimerSet::resetTimerfd\n");
    }
}

void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_DEBUG("TimerSet::readTimerfd howmany: %lu at %s\n", howmany, now.toString().c_str());
    (void)now;
    if (n != sizeof howmany) {
        LOG_ERROR("TimerSet::readTimerfd reads %zd bytes instead of 8\n", n);
    }
}

TimerSet::TimerSet(EventLoop* loop)
: m_loop(loop)
 , m_timerfd(createTimerFd())
 , m_timerfdChannel(loop, m_timerfd)
 , m_timers()
 , m_callingExpiredTimers(false)
 , m_cancelingTimers() {

    m_timerfdChannel.setReadCallback(std::bind(&TimerSet::handleRead, this));
    m_timerfdChannel.enableReading();
}

TimerSet::~TimerSet() {
    m_timerfdChannel.disableAll();
    m_timerfdChannel.remove();
    ::close(m_timerfd);
}

TimerId TimerSet::addTimer(TimerCallback cb, Timestamp when, double interval) {
    std::shared_ptr<Timer> timer = std::make_shared<Timer>(std::move(cb), when, interval);
    m_loop->runInLoop(std::bind(&TimerSet::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerSet::cancel(TimerId timer_id) {
    m_loop->runInLoop(std::bind(&TimerSet::cancelInLoop, this, timer_id));   
}

void TimerSet::addTimerInLoop(std::shared_ptr<Timer> timer) {
    m_loop->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        resetTimerfd(m_timerfd, timer->expiration());
    }
}

void TimerSet::cancelInLoop(TimerId timer_id) {
    m_loop->assertInLoopThread();
    assert(m_timers.size() == m_activeTimers.size());

    ActiveTimer activetimer(timer_id.m_timer, timer_id.m_sequence);
    ActiveTimerSet::iterator it = m_activeTimers.find(activetimer);
    if (it != m_activeTimers.end()) {
        size_t n = m_timers.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);(void)n;
        m_activeTimers.erase(activetimer);
    } else {
        m_cancelingTimers.insert(activetimer);
    }
    assert(m_timers.size() == m_activeTimers.size());
}

void TimerSet::handleRead() {
    m_loop->assertInLoopThread();
    Timestamp now = Timestamp::now();
    readTimerfd(m_timerfd, now);

    std::vector<Entry> expired = getExpired(now);
    m_callingExpiredTimers = true;
    m_cancelingTimers.clear();
    for (const Entry& it : expired) {
        it.second->run();
    }
    m_callingExpiredTimers = false;

    reset(expired, now);
}

std::vector<TimerSet::Entry> TimerSet::getExpired(Timestamp now) {
    assert(m_timers.size() == m_activeTimers.size());

    std::vector<Entry> expiredList;

    Entry sentry(now, std::make_shared<Timer>(nullptr, Timestamp::invalid(), 0.0));
    TimerList::iterator end = m_timers.lower_bound(sentry);
    assert(end == m_timers.end() || now < end->first);

    expiredList.reserve(expiredList.size() + std::distance(m_timers.begin(), end));
    expiredList.assign(m_timers.begin(), end);

    m_timers.erase(m_timers.begin(), end);
    
    for (auto &it : expiredList) {
        ActiveTimer activetimer(it.second, it.second->sequence());
        size_t n = m_activeTimers.erase(activetimer);
        assert(n == 1);(void)n;
    }

    assert(m_timers.size() == m_activeTimers.size());

    return std::move(expiredList);
}

void TimerSet::reset(const std::vector<Entry>& expired, Timestamp now) {
    Timestamp nextExpire;
    for (const Entry& it : expired) {
        ActiveTimer activetimer(it.second, it.second->sequence());
        if (it.second->repeat() 
                && m_cancelingTimers.find(activetimer) == m_cancelingTimers.end()) {
            it.second->restart(now);
            insert(it.second);
        }
    }

    if (!m_timers.empty()) {
        nextExpire = m_timers.begin()->second->expiration();
    }

    if (nextExpire.isValid()) {
        resetTimerfd(m_timerfd, nextExpire);
    }
}

bool TimerSet::insert(std::shared_ptr<Timer> timer) {
    m_loop->assertInLoopThread();
    assert(m_timers.size() == m_activeTimers.size());

    bool earliestChanged = false;
    Timestamp when = timer->expiration();

    TimerList::iterator it = m_timers.begin();
    if (it == m_timers.end() || when < it->first) {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result = m_timers.insert(Entry(when, timer));
        assert(result.second);(void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result = m_activeTimers.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);(void)result;
    }

    assert(m_timers.size() == m_activeTimers.size());

    return earliestChanged;
}