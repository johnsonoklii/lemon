#ifndef LEMON_CONSOLE_APPENDER_H_
#define LEMON_CONSOLE_APPENDER_H_

#include "lemon/base/logger/log_appender.h"

namespace lemon {
namespace log {

class ConsoleAppender : public LogAppenderInterface {
public:
    ConsoleAppender();
    ~ConsoleAppender();
    virtual void append(const char* msg) override;
};

} // namespace log
} // namespace lemon

#endif