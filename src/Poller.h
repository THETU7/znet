/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/Poller.h                                                 #
  #                                                                      #
  # @ Description: a base class for poll and epoll                       #
  ########################################################################*/

#include <map>
#include <vector>

#include <poll.h>

#include <boost/core/noncopyable.hpp>

#include "EventLoop.h"

namespace znet {
namespace reactor {
class Poller : boost::noncopyable {
public:
  using ChannelList = std::vector<Channel *>;

  Poller(EventLoop *);
  ~Poller(){};

  // Polls the I/O events
  // Must be called in loop thread;
  long poll(int timeoutMs, ChannelList *activeChannels);

  // Change the interested I/O thread
  // Must be called in loop thread;
  void updateChannel(Channel *);
  void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }
  void removeChannel(Channel *);

private:
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

  using PollFdList = std::vector<struct pollfd>;
  using ChannelMap = std::map<int, Channel *>;

  EventLoop *ownerLoop_;
  PollFdList pollfds_;
  ChannelMap channels_;
};
} // namespace reactor
} // namespace znet
