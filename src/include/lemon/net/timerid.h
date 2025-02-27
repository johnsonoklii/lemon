#ifndef LEMON_TIMEID_H
#define LEMON_TIMEID_H

#include <stdint.h>

#include <memory>

namespace lemon {
namespace net {

class Timer;

class TimerId {
public:
    TimerId(): m_timer(), m_sequence(0) {}
    TimerId(std::shared_ptr<Timer> timer, int64_t seq): m_timer(timer), m_sequence(seq) {}

    friend class TimerSet;
private:
    std::shared_ptr<Timer> m_timer;
    int64_t m_sequence;
};

} // namespace net
} // namespace lemon



#endif