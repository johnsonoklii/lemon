#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

namespace lemon {
namespace log {

enum LogLevel {
    UNKNOWN = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

} // namespace log
} // namespace lemon


#endif