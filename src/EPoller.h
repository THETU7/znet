/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/Poller.h                                                 #
  #                                                                      #
  # @ Description: a class for epoll                                     #
  ########################################################################*/

#include <map>
#include <vector>

#include <sys/epoll.h>

#include <boost/core/noncopyable.hpp>

#include "EventLoop.h"

namespace znet {
namespace reactor {
class EPoller : boost::noncopyable {
public:
  using ChannelList = std::vector<Channel *>;

  EPoller(EventLoop *);
  ~EPoller();

  // Polls the I/O events
  // Must be called in loop thread;
  long poll(int timeoutMs, ChannelList *activeChannels);

  // Change the interested I/O thread
  // Must be called in loop thread;
  void updateChannel(Channel *);
  void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }
  void removeChannel(Channel *);

private:
  static const int kInitEventListSize = 16;

  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  void update(int, Channel *);

  using EventList = std::vector<struct epoll_event>;
  using ChannelMap = std::map<int, Channel *>;

  EventLoop *ownerLoop_;
  int epollfd_;
  EventList events_;
  ChannelMap channels_;
};
} // namespace reactor
} // namespace znet
