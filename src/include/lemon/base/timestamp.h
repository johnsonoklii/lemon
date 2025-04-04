#ifndef LEMON_TIMESTAMP_H
#define LEMON_TIMESTAMP_H

#include <ctime>

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

namespace lemon {
namespace base {

class Timestamp {
public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    Timestamp();
    Timestamp(int64_t microseconds);
    static Timestamp now();
    static Timestamp addTime_Second(Timestamp timestamp, double microseconds);
    static Timestamp addTime_MilliSecond(Timestamp timestamp, double microseconds);
    static Timestamp addTime_MicroSecond(Timestamp timestamp, double microseconds);
    static Timestamp invalid();

    std::string toString() const;
    bool isValid() const {  return m_systemTimePoint.time_since_epoch().count() > 0; }   

    int64_t microSecondsSinceEpoch() const;
    int64_t milliSecondsSinceEpoch() const;
    int64_t secondsSinceEpoch() const;

    bool operator<(const Timestamp& rhs) const {
        return m_systemTimePoint < rhs.m_systemTimePoint;
    }

    bool operator==(const Timestamp& rhs) const {
        return m_systemTimePoint == rhs.m_systemTimePoint 
            || microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch()
            || milliSecondsSinceEpoch() == rhs.milliSecondsSinceEpoch()
            || secondsSinceEpoch() == rhs.secondsSinceEpoch();
    }

private:
    Timestamp(const std::chrono::system_clock::time_point& timePoint);

    std::chrono::system_clock::time_point m_systemTimePoint;
};

} // namespace base
} // namespace lemon

#endif