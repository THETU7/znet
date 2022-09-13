/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-07                                                   #
  # @ File: src/EventLoopThread.h                                        #
  #                                                                      #
  # @ Description:                                                       #
  ########################################################################*/

#ifndef REACTOR_SRC_EVENTTHREAD_H_
#define REACTOR_SRC_EVENTTHREAD_H_

#include <condition_variable>
#include <mutex>
#include <thread>

#include <boost/core/noncopyable.hpp>

#include "EventLoop.h"

namespace znet {
class EvenetThread : boost::noncopyable {
public:
  EvenetThread();
  ~EvenetThread();

  reactor::EventLoop *startLoop();

private:
  void threadFun();

  reactor::EventLoop *loop_;
  bool existing_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::thread thread_;
};
} // namespace znet

#endif // !REACTOR_SRC_EVENTTHREAD_H_
