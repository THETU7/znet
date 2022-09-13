/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/EventLoop.h                                              #
  #                                                                      #
  # @ Description: a class define for Event Loop                         #
  ########################################################################*/

#ifndef REACTOR_SRC_EVENTLOOP_H_
#define REACTOR_SRC_EVENTLOOP_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/core/noncopyable.hpp>

#include "AsyncLog.h"
#include "Callbacks.h"
#include "Timer.h"
#include "TimerId.h"

namespace znet {
namespace reactor {

class Channel;
class EPoller;
class TimerQueue;

class EventLoop : boost::noncopyable {
public:
  using ChannelList = std::vector<Channel *>;
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void loop();
  /**
   * @brief if not in thread of Current Loop, it will be FATAL and EXIT
   */
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread();
  void updateChannel(Channel *);
  void removeChannel(Channel *);
  void quit() {
    quit_ = true;
    if (!isInLoopThread()) {
      wakeup();
    }
  }
  void cancel(timer::TimerId timerid);
  /**
   * @brief 在当前线程中调用，减少锁，如果不在当前线程，则直接加入 Functor
   *
   * @param cb Callback function (void())
   */
  void runInloop(const Functor &cb);

  /**
   * @brief run Callback Function in time(s)
   *
   * @param time unit(s)
   * @param cb void()
   */
  timer::TimerId runAt(const timer::Timestamp time,
                       const timer::TimerCallBack &cb);
  // delay 单位为 s
  timer::TimerId runAfter(double delay, const timer::TimerCallBack &cb);
  /**
   * @brief run CallBack function ervery interval s
   *
   * @param interval time interval(s)
   * @param cb void() function
   */
  timer::TimerId runEvery(double interval, const timer::TimerCallBack &cb);

  /**
   * @brief insert CallBack Function to LoopQueue
   *
   * @param cb std::function<void()> callBackFunction
   */
  void queueInLoop(const Functor &cb);
  // for wakeup event
  void handleRead();

private:
  void wakeup();
  void abortNotInLoopThread();
  void doPendingFunctors_();

  bool looping_;
  bool quit_;
  bool callingPandingFunctors_;
  const pid_t tid_;
  ChannelList activeChannels;
  // std::unique_ptr<Poller> poller_;
  std::unique_ptr<EPoller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
  std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;
};
} // namespace reactor
} // namespace znet

#endif // !REACTOR_SRC_EVENTLOOP_H_
