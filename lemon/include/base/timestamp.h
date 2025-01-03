#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace lemon {
namespace base {

class Timestamp {
public:
    static Timestamp now();
    std::string toString() const;

private:
    Timestamp(const std::chrono::system_clock::time_point& timePoint);

    std::chrono::system_clock::time_point systemTimePoint_;
};

} // namespace base
} // namespace lemon

#endif