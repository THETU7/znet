#include "Channel.h"

#include <poll.h>

namespace znet {
namespace reactor {

const int Channel::kNoneEvent = 0;
// POLLIN : 有数据需要读取
// POLLPRI: 异常
const int Channel::kReadEvent = POLLIN | POLLPRI;
// POLLOUT: 可写
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, const int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1),
      eventHanding_(false), readCallBack_(), writeCallBack_(),
      errorCallBack_() {
  LOGTARCE << "Created Channel eventfd:" << fd;
}

Channel::~Channel() { assert(!eventHanding_); }

void Channel::handleEvent(timer::Timestamp time) {
  eventHanding_ = true;
  if (revents_ & POLLNVAL) {
    LOGWARN << "Channel::handle_event() POLLNVAL, maybe not open fd:" << fd_;
  }

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOGWARN << "Channel::handle_event() POLLHUP";
    if (closeCallBack_)
      closeCallBack_();
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallBack_)
      errorCallBack_();
  }

  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallBack_) {
      readCallBack_(time);
    }
  }

  if (revents_ & POLLOUT) {
    if (writeCallBack_)
      writeCallBack_();
  }
  eventHanding_ = false;
}

void Channel::update() { loop_->updateChannel(this); }

} // namespace reactor
} // namespace znet
