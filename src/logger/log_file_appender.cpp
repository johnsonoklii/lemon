#include "lemon/logger/log_file_appender.h"
#include "lemon/base/utils.h"

#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>

#include <stdexcept>

using namespace lemon;
using namespace lemon::log;

const int kSumLength = 1024;
thread_local char t_filename[kSumLength];

LogFileAppender::LogFileAppender(const char* file_name, bool async, int roll_size, int flush_interval, int check_everyn, bool thread_safe)
    : m_file_name(file_name)
        , m_async(async)
        , m_flush_interval(flush_interval)
        , m_roll_size(roll_size)
        , m_thread_safe(thread_safe)
        , m_writen_bytes(0)
        , m_count(0)
        , m_check_everyn(check_everyn)
        , m_last_period(0)
        , m_last_roll(0)
        , m_last_flush(0)
        , m_file(nullptr)
        , m_async_logging(nullptr)
        , m_mutex(m_thread_safe ? new std::mutex() : nullptr) {
    roll_file();

    if (m_async) {
        m_async_logging = new AsyncLogging(std::bind(&LogFileAppender::fwrite, this, std::placeholders::_1)
                                                , m_flush_interval);
        m_async_logging->setFlushCallback(std::bind(&LogFileAppender::fflush, this));
    }
}

LogFileAppender::~LogFileAppender() {
    // first free async logging, because it will call fwrite in thread
    if (m_async_logging) {
        delete m_async_logging;
    }

    close();
}

void LogFileAppender::append(const char* msg) {
    if (m_async) {
        m_async_logging->pushMsg(inner_message{msg});
    } else {
        fwrite(msg);
    }
}

void LogFileAppender::fwrite(const char* msg) {
    if (m_mutex) {
        std::lock_guard<std::mutex> lock(*m_mutex);
        fwrite_unlocked(msg);
    } else {
        fwrite_unlocked(msg);
    }
}

void LogFileAppender::fwrite_unlocked(const char* msg) {
    assert(m_file);

    // FIXME: 可以按照大小滚动，默认会按天滚动
    
    time_t now = ::time(nullptr);
    roll_file_byday(now);
    
    write(msg, strlen(msg));

    roll_file_bysize();

    check(now);
}

void LogFileAppender::write(const char* msg, size_t len) {
    size_t written = 0;
    while (written < len) {
        size_t remain = len - written;
        size_t n = 0;
        #if !defined(__linux__)
            n = ::fwrite(msg + written, 1, remain, m_file);
        #else
            n = ::fwrite_unlocked(msg + written, 1, remain, m_file);
        #endif
        if (n != remain) {
            int err = ferror(m_file);
            if (err) {
                fprintf(stderr, "LogFileAppender::write() failed %s\n", base::Util::get_err_info(err));
                break;
            }
            if (n == 0) { throw std::runtime_error("write failed, FILE* is null\n"); }
        }
        written += n;
    }

    m_writen_bytes += written;
}

void LogFileAppender::fflush() {
    if (m_mutex) {
        std::lock_guard<std::mutex> lock(*m_mutex);
        fflush_unlocked();
    } else {
        fflush_unlocked();
    }
}

void LogFileAppender::fflush_unlocked(const time_t* cache_now) {
    if (m_file) {
        time_t now;
        if (cache_now != nullptr) { now = *cache_now; } 
        else { now = ::time(nullptr); }

        ::fflush(m_file);
        m_last_flush = now;
    }
}

void LogFileAppender::roll_file(const time_t* cache_now) {
    time_t now;
    if (cache_now != nullptr) { now = *cache_now; } 
    else { now = ::time(nullptr); }

    if (now > m_last_roll) {    
        auto filename = get_log_file_name(now);
        mk_new_file(filename);

        auto start = now / kRollPerSeconds * kRollPerSeconds;  // 更新天的数据
        m_last_roll   = now;
        m_last_flush  = now;
        m_last_period = start;
    }
}

void LogFileAppender::roll_file_byday(time_t& now) {
    time_t cur_period = now / kRollPerSeconds * kRollPerSeconds;
    if (cur_period != m_last_period) {
        roll_file(&now);
    } 
}

void LogFileAppender::roll_file_bysize() {
    if (m_writen_bytes > m_roll_size) {
        roll_file();
        reset_written();
    } 
}

void LogFileAppender::check(time_t& now) {
    ++m_count;
    if (m_count >= m_check_everyn) {
        m_count         = 0;
        if (now - m_last_flush > m_flush_interval) {
            fflush_unlocked(&now);
        }
    }
}

const char* LogFileAppender::get_log_file_name(time_t& now) {
    size_t base_sz = ::strlen(m_file_name);
    char* filename = t_filename;
    strcpy(t_filename, m_file_name);

    size_t tsz = strftime(t_filename + base_sz, sizeof(t_filename) - base_sz, ".%Y%m%d-%H%M%S", localtime(&now));
    snprintf(t_filename + base_sz + tsz, sizeof(t_filename) - base_sz - tsz, ".%s.%d.log", base::ProcessInfo::get_host_name(),
                              static_cast<int>( base::ProcessInfo::get_pid()));

    return filename;
}

void LogFileAppender::mk_new_file(const char* file_name) {
    close();
    // FIXME: 需要判断是否存在文件夹
    m_file = fopen(file_name, "a");
}

void LogFileAppender::close() {
    if (m_file) {
        if (::fflush(m_file) == 0) {
            fclose(m_file);
        } else {
            fprintf(stderr, "Error flushing file\n");
        }
    } 
}