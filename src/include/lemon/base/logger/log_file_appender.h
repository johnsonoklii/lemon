#ifndef LEMON_LOG_FILE_APPENDER_H_
#define LEMON_LOG_FILE_APPENDER_H_

#include <memory>
#include <mutex>

#include "lemon/base/logger/log_appender.h"
#include "lemon/base/logger/async_logging.h"

namespace lemon {
namespace log {
    
class LogFileAppender : public LogAppenderInterface
                        , public std::enable_shared_from_this<LogFileAppender> {
public:
    LogFileAppender(const char* file_name, bool is_async, int roll_size, int flush_interval, int check_everyn = 1024, bool thread_safe = false);
    ~LogFileAppender();
    virtual void append(const char* msg) override;

    void fwrite(const char* msg);
    void fflush();

private:
    enum { kRollPerSeconds = 60 * 60 * 24 };

    void fwriteUnlocked(const char* msg);
    void fflushUnlocked(const time_t* cache_now = nullptr);
    void write(const char* msg, size_t len);

    void rollFile(const time_t* cache_now = nullptr);
    void rollFileByDay(time_t& now);
    void rollFileBySize();
    void check(time_t& now);

    void resetWritten() { m_writen_bytes = 0; }

    void mkNewFile(const char* file_name);

    void close();

    const char* getLogFileName(time_t& now);

    const char* m_file_name;
    bool m_async;
    int m_flush_interval;
    size_t m_roll_size;

    bool m_thread_safe;
    size_t m_writen_bytes;
    int m_count;   // 统计log次数
    int m_check_everyn;
    time_t m_last_period;
    time_t m_last_roll;
    time_t m_last_flush;

    FILE* m_file;
    AsyncLogging* m_async_logging;

    std::unique_ptr<std::mutex> m_mutex;
};

} // namespace log
} // namespace lemon

#endif