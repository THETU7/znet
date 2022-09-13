/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-09                                                   #
  # @ File: src/EventLoopThreadPool.h                                    #
  #                                                                      #
  # @ Description: Event Loop Poll                                       #
  ########################################################################*/

#ifndef REACTOR_SRC_EVENTLOOPTHREADPOLL
#define REACTOR_SRC_EVENTLOOPTHREADPOLL

#include "EventLoop.h"

#include <memory>
#include <vector>

#include <boost/core/noncopyable.hpp>

#include "EventLoopThread.h"

namespace znet {
using EventLoop = reactor::EventLoop;
class EventLoopThreadPool : public boost::noncopyable {
public:
  using EvenetThreadPtr = std::unique_ptr<EvenetThread>;

  EventLoopThreadPool(EventLoop *baseloop);
  ~EventLoopThreadPool();
  void setThreadNums(int numThreads) { numThreads_ = numThreads; }
  void start();
  EventLoop *getNextLoop();

private:
  EventLoop *baseloop_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<EvenetThreadPtr> threads_;
  std::vector<EventLoop *> loops_;
};
} // namespace znet

#endif // !REACTOR_SRC_EVENTLOOPTHREADPOLL
