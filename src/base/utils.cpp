#include "lemon/base/utils.h"

#include <cstring>
#include <unistd.h>
#include <sys/syscall.h>

namespace lemon {
namespace base {

thread_local char           t_errnobuf[512];
thread_local int            t_cached_tid = 0;

const char* Util::getErrInfo(int error_code)
{
   auto p = ::strerror_r(error_code, t_errnobuf, sizeof(t_errnobuf));
   (void)p;
   return t_errnobuf;
}

const char* ProcessInfo::getHostName() {
   thread_local char buf[256]{};
   if (buf[0] == -1) { return buf + 1; }
   if (::gethostname(buf + 1, sizeof(buf) - 1) == 0)
   {
      buf[0] = -1;
      return buf + 1;
   }
   return "unknownhost";
}

pid_t ProcessInfo::tid() {
   if (t_cached_tid == 0)
   {
      t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
   }
   return t_cached_tid;
}

}
}
