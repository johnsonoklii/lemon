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

Timestamp Timestamp::addTime(Timestamp timestamp, double second) {
    int64_t microseconds = second * kMicroSecondsPerSecond;
    return Timestamp(timestamp.m_systemTimePoint + std::chrono::microseconds(static_cast<long long>(microseconds)));
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