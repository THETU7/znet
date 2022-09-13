/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: test/test03.cc                                               #
  #                                                                      #
  # @ Description: a RAII class for timerid                              #
  ########################################################################*/

#ifndef REACTOR_SRC_TIMERID_H_
#define REACTOR_SRC_TIMERID_H_

#include "Timer.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace znet {
namespace reactor {
class TimerQueue;
}
namespace timer {
class TimerId {
public:
  TimerId() : timer_(nullptr), sequence_(0) {}
  TimerId(Timer *timer, int64_t se = 0) : timer_(timer), sequence_(se) {}
  TimerId(const TimerId &rhs) = default;
  TimerId(TimerId &&rhs) {
    timer_ = rhs.timer_;
    rhs.timer_ = nullptr;
  }
  TimerId operator=(const TimerId &rhs) { return TimerId(rhs); }

  friend class reactor::TimerQueue;

private:
  Timer *timer_;
  int64_t sequence_;
};
} // namespace timer
} // namespace znet

#endif // !REACTOR_SRC_TIMERID_H_
