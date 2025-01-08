#include "lemon/logger/log_config.h"

using namespace lemon;
using namespace lemon::log;

LogConfig::LogConfig(LogLevel level, const char* file, bool async, bool console, int flush_interval, size_t roll_size)
: m_level(level)
    , m_file(file)
    , m_async(async)
    , m_console(console)
    , m_flush_interval(flush_interval)
    , m_rool_size(roll_size) {
}

LogConfig::~LogConfig() {

}

LogConfig& LogConfig::getInstance() {
    static LogConfig config;
    return config;
}