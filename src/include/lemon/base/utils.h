#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

namespace lemon {
namespace base {

struct Util
{
    static const char* get_err_info(int error_code);
};

struct ProcessInfo {
    static const char* get_host_name();
    static pid_t get_pid();
};


} // namespace base  
} // namespace lemon

#endif