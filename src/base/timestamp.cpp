#include "lemon/base/timestamp.h"

using namespace lemon;
using namespace lemon::base;

Timestamp::Timestamp() 
: m_systemTimePoint() {
}

Timestamp::Timestamp(const std::chrono::system_clock::time_point& timePoint) 
: m_systemTimePoint(timePoint) {
}

Timestamp Timestamp::now() {
    return Timestamp(std::chrono::system_clock::now());
}

std::string Timestamp::toString() const {
    std::time_t time = std::chrono::system_clock::to_time_t(m_systemTimePoint);
    std::tm bt = *std::localtime(&time);

    char buffer[32] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &bt);
    return buffer;
}
