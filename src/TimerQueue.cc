#include "TimerQueue.h"
#include "Callbacks.h"

#include <assert.h>
#include <bits/types/struct_itimerspec.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <utility>

namespace znet {
namespace reactor {

int createTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOGFATAL << "Fail to create Timerfd";
  }
  return timerfd;
}

struct timespec howMuchTimeFromNow(timer::Timestamp timestamp) {
  timer::Timestamp now =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count();

  struct timespec t;
  t.tv_sec = (timestamp - now) / (1000 * 1000);
  t.tv_nsec = ((timestamp - now) % (1000 * 1000)) * 1000;
  return t;
}

void resetTimerfd(int timerfd, timer::Timestamp when) {
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(when);
  LOGTARCE << "resetTimerfd:" << timerfd
           << " value:" << newValue.it_value.tv_sec << "."
           << newValue.it_value.tv_nsec;
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    LOGFATAL << "ResetTimerfd Error";
  }
}

void readTimerfd(int timerfd, timer::Timestamp now) {
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOGTARCE << "TimerQueue::handleRead() " << howmany << " at " << now;
  if (n != sizeof howmany) {
    LOGERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timerfd_(createTimerfd()), timerIdChannel_(loop, timerfd_),
      callingExpriedTimers_(false) {
  LOGTARCE << "Create Timerfd " << timerfd_;
  timerIdChannel_.setReadCallBack(
      [ptr = this](timer::Timestamp time) { ptr->handleRead(); });
  timerIdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  ::close(timerfd_);
  for (auto it : timers_) {
    delete it.second;
  }
}

void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  timer::Timestamp now =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count();
  readTimerfd(timerfd_, now);
  std::vector<Entity> expired = getExpired(now);
  callingExpriedTimers_ = true;
  cancelingTimers_.clear();
  for (auto it : expired) {
    it.second->run();
  }
  callingExpriedTimers_ = false;
  reset(expired, now);
}

std::vector<TimerQueue::Entity> TimerQueue::getExpired(timer::Timestamp now) {
  std::vector<Entity> expired;
  Entity sentity =
      std::make_pair(now, reinterpret_cast<timer::Timer *>(UINTPTR_MAX));

  auto it = timers_.lower_bound(sentity);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, std::back_inserter(expired));
  timers_.erase(timers_.begin(), it);

  for (auto &entry : expired) {
    ActiveTimer timer(entry.second, entry.second->sequence());
    size_t n = activeTimers_.erase(timer);
    assert(n == 1);
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entity> &expried,
                       timer::Timestamp now) {
  timer::Timestamp nextExpire = 0;
  for (auto it : expried) {
    ActiveTimer timer(it.second, it.second->sequence());
    if (it.second->repeat() &&
        cancelingTimers_.find(timer) == cancelingTimers_.end()) {
      it.second->restart(now);
      insert(it.second);
    } else {
      delete it.second;
    }
  }

  if (!timers_.empty()) {
    nextExpire = timers_.begin()->second->expiration();
  }
  if (nextExpire > now) {
    resetTimerfd(timerfd_, nextExpire);
    // reset(timers_, nextExpire);
  }
}

bool TimerQueue::insert(timer::Timer *timer) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  bool earliestChanged = false;
  timer::Timestamp when = timer->expiration();
  auto it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliestChanged = true;
  }
  {
    auto re = timers_.insert(std::make_pair(when, timer));
    assert(re.second);
  }
  {
    auto re = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(re.second);
  }
  assert(timers_.size() == activeTimers_.size());
  return earliestChanged;
}

timer::TimerId TimerQueue::addTimer(const timer::TimerCallBack &cb,
                                    timer::Timestamp when, double interval) {
  timer::Timer *timer = new timer::Timer(cb, when, interval);
  loop_->runInloop(
      [ptr = this, timer = timer]() { ptr->addTimerInLoop(timer); });
  return timer::TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(timer::Timer *timer) {
  loop_->assertInLoopThread();

  bool eraliestChanged = insert(timer);

  if (eraliestChanged) {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::cancel(timer::TimerId timerId) {
  loop_->runInloop(
      [ptr = this, timerId = timerId]() { ptr->cancelInLoop(timerId); });
}

void TimerQueue::cancelInLoop(timer::TimerId timerId) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer timer(timerId.timer_, timerId.sequence_);
  auto it = activeTimers_.find(timer);
  if (it != activeTimers_.end()) {
    size_t n = timers_.erase(Entity(it->first->expiration(), it->first));
    assert(n == 1);
    delete it->first;
    activeTimers_.erase(it);
  } else if (callingExpriedTimers_) {
    cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}

} // namespace reactor
} // namespace znet
