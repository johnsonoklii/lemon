#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H

#include <memory>

#include "lemon/include/logger/log_level.h"

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
    LogLevel m_level;
    const char* m_file;
    bool m_async;
    bool m_console;
    int m_flush_interval;
    size_t m_rool_size;
};

#define GLOB_CONFIG lemon::log::LogConfig::getInstance()

} // namespace log
} // namespace lemon

#endif