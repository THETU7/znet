#include "EventLoop.h"

#include <assert.h>
#include <poll.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstdlib>
#include <memory>

#include "Callbacks.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "EPoller.h"
#include "Poller.h"
#include "TimerQueue.h"

namespace znet {
namespace reactor {

// 只有在该线程EventLoop初始化后才不为空，但在该线程中第二次初始化 EVENTLOOP 会
// FATAL
__thread EventLoop *t_loopInThisThread = 0;

int createEventfd() {
  int event = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event < 0) {
    LOGFATAL << "create eventfd fafiled";
  }
  return event;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), callingPandingFunctors_(false),
      tid_(CurrentThread::tid()), poller_(std::make_unique<EPoller>(this)),
      timerQueue_(std::make_unique<TimerQueue>(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)) {
  LOGTARCE << "Event Loop created " << this << " in thread " << tid_;
  if (t_loopInThisThread) {
    LOGFATAL << "Another EventLoop " << t_loopInThisThread
             << " exists in this thread " << tid_;
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallBack([ptr = this](timer::Timestamp time) {
    LOGTARCE << "Call Back";
    ptr->handleRead();
  });
  wakeupChannel_->enableReading();
}

bool EventLoop::isInLoopThread() { return tid_ == CurrentThread::tid(); }

void EventLoop::abortNotInLoopThread() {
  if (!isInLoopThread()) {
    LOGFATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
             << " was created in threadId_ = " << tid_
             << ", current thread id = " << CurrentThread::tid();
  }
}

EventLoop::~EventLoop() {
  assert(!looping_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();

  looping_ = true;
  quit_ = false;
  while (!quit_) {
    activeChannels.clear();
    timer::Timestamp time = poller_->poll(10 * 1000, &activeChannels);
    LOGDEBUG << time;
    for (auto it : activeChannels) {
      (*it).handleEvent(time);
    }
    // 减少因为 handleEvent 调用
    // runinloop,导致wakeup的多次调用，这样当handleEvent 执行完成后就直接
    // doPendingFunctors_
    doPendingFunctors_();
  }

  LOGTARCE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::updateChannel(Channel *ch) { poller_->updateChannel(ch); }
void EventLoop::removeChannel(Channel *ch) {
  assert(ch->owerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(ch);
}

timer::TimerId EventLoop::runAt(const timer::Timestamp time,
                                const timer::TimerCallBack &cb) {
  timer::Timestamp mictime = time * 1000 * 1000;
  return timerQueue_->addTimer(cb, mictime, 0);
}

timer::TimerId EventLoop::runAfter(double delay,
                                   const timer::TimerCallBack &cb) {
  timer::Timestamp now =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count();

  timer::Timestamp time = timer::addTime(now, delay);
  return timerQueue_->addTimer(cb, time, 0);
}

timer::TimerId EventLoop::runEvery(double interval,
                                   const timer::TimerCallBack &cb) {
  timer::Timestamp now =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count();

  timer::Timestamp time = timer::addTime(now, interval);
  return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::queueInLoop(const Functor &cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(cb);
  }

  // callingPandingFunctors_ is ture Only happen in run function of CallBack
  // callingPandingFunctors_ 只会在 doPendingFunctors_ 调用回调函数时才会发生
  if (!isInLoopThread() || callingPandingFunctors_) {
    wakeup();
  }
}

void EventLoop::doPendingFunctors_() {
  std::vector<Functor> functors;
  callingPandingFunctors_ = true;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (auto &it : functors) {
    it();
  }
  callingPandingFunctors_ = false;
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  size_t n = ::write(wakeupFd_, &one, sizeof one);
  LOGTARCE << "EventLoop::wakeup() write " << wakeupFd_ << " writes " << n;
  if (n != sizeof one) {
    LOGERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  LOGTARCE << "EventLoop::handleRead() read " << wakeupFd_ << " reads " << n;
  if (n != sizeof one) {
    LOGERROR << "EventLoop::wakeup() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::runInloop(const Functor &cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::cancel(timer::TimerId timerid) { timerQueue_->cancel(timerid); }

class IgnoreSigPipe {
public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

// 全局变量，为了消除 SIGPIPE对server的影响
IgnoreSigPipe initObj;

} // namespace reactor
} // namespace znet
