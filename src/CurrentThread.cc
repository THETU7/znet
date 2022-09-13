#include "CurrentThread.h"

#include <cstdio>
#include <stdlib.h>
#include <syscall.h>
#include <type_traits>
#include <unistd.h>

namespace znet {
namespace CurrentThread {
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char *t_threadName = "unknown";

static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

void cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5d", t_cachedTid);
  }
}

bool isMainThread() { return tid() == ::getpid(); }

} // namespace CurrentThread
} // namespace znet
