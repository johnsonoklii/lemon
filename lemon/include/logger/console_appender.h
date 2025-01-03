#ifndef CONSOLE_APPENDER_H_
#define CONSOLE_APPENDER_H_

#include "lemon/include/logger/log_appender.h"

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