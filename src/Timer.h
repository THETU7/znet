/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/Timer.h                                                  #
  #                                                                      #
  # @ Description: a entity class for change data                        #
  ########################################################################*/
#ifndef REACTOR_SRC_TIMER_H_
#define REACTOR_SRC_TIMER_H_

#include "AsyncLog.h"
#include "Callbacks.h"
#include "Timestamp.h"

#include <atomic>

namespace znet {
namespace timer {
class Timer {
public:
  Timer(const TimerCallBack &cb, Timestamp expiration, const double interval)
      : callback_(cb), expiration_(expiration), interval_(interval),
        repeat_(interval > 0.0), sequence_(s_numCreated++) {
    LOGTARCE << "repeated: " << repeat_;
  };
  Timer(const Timer &) = default;

  void restart(Timestamp now);

  void run() const { callback_(); }
  Timestamp expiration() { return expiration_; }
  bool repeat() { return repeat_; }
  double interval() { return interval_; }
  int64_t sequence() { return sequence_; }

  ~Timer() {}

private:
  const TimerCallBack callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
  const int64_t sequence_;

  static std::atomic_int64_t s_numCreated;
};
} // namespace timer
} // namespace znet

#endif // !REACTOR_SRC_TIMER_H_
