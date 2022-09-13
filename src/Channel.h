/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/Channel.h                                                #
  #                                                                      #
  # @ Description: a class for register event                            #
  ########################################################################*/

#ifndef REACTOR_SRC_CHANNEL_H_
#define REACTOR_SRC_CHANNEL_H_

#include "Callbacks.h"
#include "assert.h"

#include <functional>

#include <boost/core/noncopyable.hpp>

#include "EventLoop.h"

namespace znet {
namespace reactor {

class Channel : boost::noncopyable {
public:
  using EventCallBack = std::function<void()>;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent(timer::Timestamp time);

  int fd() { return fd_; }
  int events() { return events_; }

  void setIndex(int index) { index_ = index; }
  void setReadCallBack(const ReadEventCallBack &cb) { readCallBack_ = cb; }
  void setWriteCallBack(const EventCallBack &cb) { writeCallBack_ = cb; }
  void setErrorCallBack(const EventCallBack &cb) { errorCallBack_ = cb; }
  void setCloseCallBack(const EventCallBack &cb) { closeCallBack_ = cb; }
  void set_revents(int revt) { revents_ = revt; }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }

  void enableWrite() {
    events_ |= kWriteEvent;
    update();
  }

  void disableWrite() {
    events_ &= ~kWriteEvent;
    update();
  }

  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isWriting() const { return events_ & kWriteEvent; }

  // for poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop *owerLoop() { return loop_; }

  bool isNoneEvent() { return events_ == kNoneEvent; }

private:
  void update();

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop *loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;
  bool eventHanding_;
  ReadEventCallBack readCallBack_;
  EventCallBack writeCallBack_;
  EventCallBack errorCallBack_;
  EventCallBack closeCallBack_;
};
} // namespace reactor
} // namespace znet

#endif // !REACTOR_SRC_CHANNEL_H_
