#ifndef LEMON_UTILS_H
#define LEMON_UTILS_H

#include <string>

#include <sys/types.h>

namespace lemon {
namespace base {

extern thread_local int t_cached_tid;

struct Util
{
    static const char* getErrInfo(int error_code);
};

struct ProcessInfo {
    static const char* getHostName();
    static pid_t tid();
};

struct StringUtil {
    static std::string trim(const std::string& str);
};


} // namespace base  
} // namespace lemon

#endif