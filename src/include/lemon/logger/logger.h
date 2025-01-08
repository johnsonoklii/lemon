#ifndef LEMON_LOGGER_H
#define LEMON_LOGGER_H

#include <string>
#include <unordered_map>

#include "lemon/logger/log_level.h"
#include "lemon/logger/log_appender.h"
#include "lemon/logger/log_config.h"

namespace lemon {
namespace log {

class Logger {
public:
    Logger(const char* name);
    ~Logger();

    static Logger& getInstance();
    void log(LogLevel level, const char* file_name, int line, const char* msg);

    void setConfig(LogConfig::Ptr config);

private:
    // only for singleton
    Logger();
    void init();

    const char* getLevelStr() const;
    LogLevel getLevel() const;
    bool isConsole() const;
    bool isAsync() const;
    const char* getFile() const;
    int getFlushInterval() const;
    int getRollSize() const;

    const char*     m_name;     
    LogConfig::Ptr  m_config;   // 不使用全局配置，自定义配置
    
    std::unordered_map<std::string, LogAppenderInterface::Ptr> m_appenders;
};

#define LOG_INFO(fmt, ...) \
    do { \
        char buf[1024]; \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        Logger::getInstance().log(LogLevel::INFO, __FILE__, __LINE__, buf); \
    } while (0)

} // namespace log
} // namespace lemon

#endif