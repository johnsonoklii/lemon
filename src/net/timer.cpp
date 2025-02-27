#include "lemon/net/timer.h"

using namespace lemon;
using namespace lemon::net;

thread_local int64_t t_numTimer = 0;

Timer::Timer(TimerCallback cb, Timestamp when, double interval)
: m_cb(std::move(cb))
 , m_expiration(when)
 , m_interval(interval)
 , m_repeat(m_interval > 0.0)
 , m_sequence(++t_numTimer) {

}

Timer::~Timer() {
}

void Timer::run() {
    m_cb();
}

void Timer::restart(Timestamp now) {
    if (m_repeat) {
        m_expiration = Timestamp::addTime(now, m_interval);
    }
    else {
        m_expiration = Timestamp::invalid();
    }
}
