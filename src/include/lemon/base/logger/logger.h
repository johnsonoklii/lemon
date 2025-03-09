#ifndef LEMON_LOGGER_H
#define LEMON_LOGGER_H

#include "lemon/base/logger/log_level.h"
#include "lemon/base/logger/log_appender.h"
#include "lemon/base/logger/log_config.h"

#include "lemon/base/utils.h"

#include <string>
#include <unordered_map>

namespace lemon {
namespace log {

using namespace base;

class Logger {
public:
    Logger(const char* name);
    ~Logger();

    static Logger& getInstance();
    void log(LogLevel level, const char* file_name, int line, int thread_id,const char* msg);

    void setConfig(LogConfig::Ptr config);

private:
    // only for singleton
    Logger();
    void init();

    const char* getLevelStr(LogLevel level) const;
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

#ifdef LEMON_DEBUG
#define LOG_DEBUG(fmt, ...) \
    do { \
        GLOB_LOG_CONFIG.setLevel(LogLevel::DEBUG); \
        char buf[1024]; \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        Logger::getInstance().log(LogLevel::DEBUG, __FILE__, __LINE__, ProcessInfo::tid(), buf); \
    } while (0)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#define LOG_INFO(fmt, ...) \
    do { \
        char buf[1024]; \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        Logger::getInstance().log(LogLevel::INFO, __FILE__, __LINE__, ProcessInfo::tid(), buf); \
    } while (0)

#define LOG_WARN(fmt, ...) \
    do { \
        char buf[1024]; \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        Logger::getInstance().log(LogLevel::WARN, __FILE__, __LINE__, ProcessInfo::tid(), buf); \
    } while (0)

#define LOG_ERROR(fmt, ...) \
    do { \
        char buf[1024]; \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        Logger::getInstance().log(LogLevel::ERROR, __FILE__, __LINE__, ProcessInfo::tid(), buf); \
    } while (0)

#define LOG_FATAL(fmt, ...) \
    do { \
        char buf[1024]; \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
        Logger::getInstance().log(LogLevel::FATAL, __FILE__, __LINE__, ProcessInfo::tid(), buf); \
        abort(); \
    } while (0)

} // namespace log
} // namespace lemon

#endif