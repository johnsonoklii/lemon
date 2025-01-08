#ifndef LEMON_CONSOLE_APPENDER_H_
#define LEMON_CONSOLE_APPENDER_H_

#include "lemon/logger/log_appender.h"

namespace lemon {
namespace log {

class ConsoleAppender : public LogAppenderInterface {
public:
    ConsoleAppender();
    ~ConsoleAppender();
    virtual void append(const char* msg) override;
};

} // log
} // namespace lemon

#endif