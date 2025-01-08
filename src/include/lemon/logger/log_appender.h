#ifndef LEMON_LOG_APPENDER_H
#define LEMON_LOG_APPENDER_H

#include <unistd.h>
#include <memory>

namespace lemon {
namespace log {

class LogAppenderInterface {
public:
    using Ptr = std::shared_ptr<LogAppenderInterface>;

    virtual ~LogAppenderInterface() {}
    virtual void append(const char* msg) = 0;
};

} // namespace log
} // namespace lemon

#endif