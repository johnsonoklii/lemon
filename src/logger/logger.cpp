#include "lemon/logger/logger.h"
#include "lemon/logger/console_appender.h"
#include "lemon/logger/log_file_appender.h"
#include "lemon/base/timestamp.h"

#include <cstring>

using namespace lemon;
using namespace lemon::log;

Logger::Logger()
: m_name("global") {
    init();
}

Logger::Logger(const char* name): m_name(name) {
}

Logger::~Logger() {
}

void Logger::init() {
    if (isConsole()) {
        m_appenders["console"] = LogAppenderInterface::Ptr(new ConsoleAppender());
    }
    
    const char* file = getFile();
    if (file != nullptr && strcmp(file, "") != 0) {
        m_appenders["file"] = LogAppenderInterface::Ptr(new LogFileAppender(getFile(), isAsync(), getRollSize(), getFlushInterval(), false));
    }
}

Logger& Logger::getInstance() {
    static Logger logger;
    return logger;
}

void Logger::log(LogLevel level, const char* file_name, int line, int thread_id, const char* msg) {
    if (level < getLevel()) {
        return;
    }

    // FIXME: 由一个formatter类统一处理, 移到子线程中处理
    char buffer[strlen(msg)+200];
    snprintf(buffer, sizeof(buffer), "[%s][%s][%s][%s:%d][thread:%d] %s"
                                    , base::Timestamp::now().toString().data()
                                    , m_name
                                    , getLevelStr()
                                    , file_name
                                    , line
                                    , thread_id
                                    , msg);

    for (auto& appender : m_appenders) {
        appender.second->append(buffer);
    }
}

void Logger::setConfig(LogConfig::Ptr config) {
    m_config = config;
    init();
}

const char* Logger::getLevelStr() const{
    LogLevel level = m_config ? m_config->getLevel() : GLOB_LOG_CONFIG.getLevel();
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARN:
            return "WARN";
        case ERROR:
            return "ERROR";
        case FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

LogLevel Logger::getLevel() const {
    if (m_config) {
        return m_config->getLevel();
    } else {
        return GLOB_LOG_CONFIG.getLevel();
    }
}

bool Logger::isConsole() const {
    if (m_config) {
        return m_config->isConsole();
    } else {
        return GLOB_LOG_CONFIG.isConsole();
    }
}

bool Logger::isAsync() const {
    if (m_config) {
        return m_config->isAsync();
    } else {
        return GLOB_LOG_CONFIG.isAsync();
    }
}

const char* Logger::getFile() const {
    if (m_config) {
        return m_config->getFile();
    } else {
        return GLOB_LOG_CONFIG.getFile();
    }
}

int Logger::getFlushInterval() const {
    if (m_config) {
        return m_config->getFlushInterval();
    } else {
        return GLOB_LOG_CONFIG.getFlushInterval();
    }
}

int Logger::getRollSize() const {
    if (m_config) {
        return m_config->getRollSize();
    } else {
        return GLOB_LOG_CONFIG.getRollSize();
    }
}