#ifndef LEMON_LOG_CONFIG_H
#define LEMON_LOG_CONFIG_H

#include <memory>

#include "lemon/logger/log_level.h"

namespace lemon {
namespace log {

class LogConfig {
public:
    using Ptr = std::shared_ptr<LogConfig>;

    LogConfig(LogLevel level = LogLevel::INFO
                , const char* file = ""
                , bool async = true
                , bool console = true
                , int flush_interval = 3
                , size_t rool_size = 1024 * 1024 * 64);

    ~LogConfig();

    static LogConfig& getInstance();

    void setLevel(LogLevel level) { m_level = level; }
    void setFile(const char* file) { m_file = file; }
    void setAsync(bool async) { m_async = async; }
    void setConsole(bool console) { m_console = console; }
    void setFlushInterval(int interval) { m_flush_interval = interval; }
    void setRollSize(size_t size) { m_rool_size = size; }

    LogLevel getLevel() const { return m_level; }
    const char* getFile() const { return m_file; }
    bool isAsync() const { return m_async; }
    bool isConsole() const { return m_console; }
    int getFlushInterval() const { return m_flush_interval; }
    size_t getRollSize() const { return m_rool_size;}

private:
    LogLevel    m_level;            // 日志级别
    const char* m_file;             // 日志输出文件，
    bool        m_async;            // 是否异步输出到文件
    bool        m_console;          // 是否输出到控制台
    int         m_flush_interval;   // 输出到文件，刷新缓存间隔
    size_t      m_rool_size;        // 日志文件滚动大小，默认会按照天滚动
};

#define GLOB_LOG_CONFIG lemon::log::LogConfig::getInstance()

} // namespace log
} // namespace lemon

#endif