/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/CurrentThread.h                                          #
  #                                                                      #
  # @ Description: some funcs for CurrentThread                          #
  ########################################################################*/

#ifndef REACTOR_SRC_CURRENTTHREAD_H_
#define REACTOR_SRC_CURRENTTHREAD_H_

namespace znet {
namespace CurrentThread {
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

void cacheTid();

inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char *tidString() { return t_tidString; }

inline int tidStringLenght() { return t_tidStringLength; }

inline const char *name() { return t_threadName; }

bool isMainThread();

// void sleepUsec(int64_t usec); // for testing

// string stackTrace(bool demangle);

} // namespace CurrentThread
} // namespace znet

#endif // !REACTOR_SRC_CURRENTTHREAD_H_
