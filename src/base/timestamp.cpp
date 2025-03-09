#include "lemon/base/timestamp.h"

using namespace lemon;
using namespace lemon::base;

Timestamp::Timestamp() 
: m_systemTimePoint() {
}

Timestamp::Timestamp(int64_t microseconds) {
    m_systemTimePoint = std::chrono::system_clock::time_point(std::chrono::microseconds(microseconds));
}

Timestamp::Timestamp(const std::chrono::system_clock::time_point& timePoint) 
: m_systemTimePoint(timePoint) {
}

Timestamp Timestamp::now() {
    return Timestamp(std::chrono::system_clock::now());
}

Timestamp Timestamp::addTime_Second(Timestamp timestamp, double second) {
    int64_t microseconds = second * kMicroSecondsPerSecond;
    return Timestamp(timestamp.m_systemTimePoint + std::chrono::microseconds(static_cast<long long>(microseconds)));
}

Timestamp Timestamp::addTime_MilliSecond(Timestamp timestamp, double millisecond) {
    int64_t microseconds = millisecond * 1000;
    return Timestamp(timestamp.m_systemTimePoint + std::chrono::microseconds(static_cast<long long>(microseconds)));
}

Timestamp Timestamp::addTime_MicroSecond(Timestamp timestamp, double microsecond) {
    return Timestamp(timestamp.m_systemTimePoint + std::chrono::microseconds(static_cast<long long>(microsecond)));
}

Timestamp Timestamp::invalid() {
    return Timestamp();
}

std::string Timestamp::toString() const {
    std::time_t time = std::chrono::system_clock::to_time_t(m_systemTimePoint);
    std::tm bt = *std::localtime(&time);

    char buffer[32] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &bt);
    return buffer;
}

int64_t Timestamp::microSecondsSinceEpoch() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(m_systemTimePoint.time_since_epoch()).count();
}

int64_t Timestamp::milliSecondsSinceEpoch() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_systemTimePoint.time_since_epoch()).count();
}
int64_t Timestamp::secondsSinceEpoch() const {
    return std::chrono::duration_cast<std::chrono::seconds>(m_systemTimePoint.time_since_epoch()).count();
}