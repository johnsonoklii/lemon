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
    Timestamp();
    static Timestamp now();
    std::string toString() const;

private:
    Timestamp(const std::chrono::system_clock::time_point& timePoint);

    std::chrono::system_clock::time_point systemTimePoint_;
};

} // namespace base
} // namespace lemon

#endif