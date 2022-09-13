/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/TimeQueue.h                                              #
  #                                                                      #
  # @ Description: timer queue class                                     #
  ########################################################################*/

#ifndef REACTOR_SRC_TIMEQUEUE_H_
#define REACTOR_SRC_TIMEQUEUE_H_

#include <set>

#include <boost/noncopyable.hpp>
#include <memory>

#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

namespace znet {
namespace reactor {
class TimerQueue : boost::noncopyable {
public:
  using Entity = std::pair<timer::Timestamp, timer::Timer *>;
  using TimerList = std::set<Entity>;

  TimerQueue(EventLoop *loop);
  ~TimerQueue();

  timer::TimerId addTimer(const timer::TimerCallBack &cb, timer::Timestamp when,
                          double interval);
  void addTimerInLoop(timer::Timer *timer);
  void handleRead();
  bool insert(timer::Timer *);
  std::vector<Entity> getExpired(timer::Timestamp now);
  void reset(const std::vector<Entity> &expried, timer::Timestamp now);
  void cancel(timer::TimerId timerId);

private:
  using ActiveTimer = std::pair<timer::Timer *, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void cancelInLoop(timer::TimerId);

  EventLoop *loop_;
  const int timerfd_;
  Channel timerIdChannel_;
  TimerList timers_;

  bool callingExpriedTimers_;
  ActiveTimerSet activeTimers_;
  ActiveTimerSet cancelingTimers_;
};
} // namespace reactor
} // namespace znet

#endif // !REACTOR_SRC_TIMEQUEUE_H_
