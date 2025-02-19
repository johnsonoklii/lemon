#include "lemon/base/utils.h"

#include <cstring>
#include <unistd.h>

using namespace lemon;
using namespace lemon::base;

thread_local char           t_errnobuf[512];

const char* Util::getErrInfo(int error_code)
{
#if defined(_WIN32)
   ::strerror_s(t_errnobuf, sizeof(t_errnobuf), error_code);
#else
   auto p = ::strerror_r(error_code, t_errnobuf, sizeof(t_errnobuf));
   (void)p;
#endif
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


pid_t ProcessInfo::getPid() {
#if defined(_WIN32)
   thread_local auto pid = GetCurrentProcessId();
#else
   thread_local auto pid = getpid();
#endif
   return pid;
}