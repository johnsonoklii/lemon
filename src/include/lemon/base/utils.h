#ifndef LEMON_UTILS_H
#define LEMON_UTILS_H

#include <sys/types.h>

namespace lemon {
namespace base {

struct Util
{
    static const char* getErrInfo(int error_code);
};

struct ProcessInfo {
    static const char* getHostName();
    static pid_t getPid();
};


} // namespace base  
} // namespace lemon

#endif