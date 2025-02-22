#ifndef LEMON_UTILS_H
#define LEMON_UTILS_H

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


} // namespace base  
} // namespace lemon

#endif