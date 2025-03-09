#ifndef LEMON_TIMER_H
#define LEMON_TIMER_H

#include "lemon/base/timestamp.h"
#include "lemon/net/callbacks.h"

#include <chrono>
#include <functional>

namespace lemon {
namespace net {

using namespace base;

class Timer {
public:
    Timer(TimerCallback cb, Timestamp when, double interval);
    ~Timer();

    Timestamp expiration() const { return m_expiration; }
    bool repeat() const { return m_repeat; }
    int64_t sequence() const { return m_sequence; }

    void run();
    void restart(Timestamp now);

private:
    TimerCallback m_cb;
    Timestamp m_expiration;
    double m_interval;  // 毫秒
    bool m_repeat;
    const int64_t m_sequence;
};

} // namespace net
} // namespace lemon

#endif