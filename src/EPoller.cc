#include "EPoller.h"

#include <assert.h>

#include "AsyncLog.h"
#include "Channel.h"

namespace znet {
namespace reactor {
namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
} // namespace

EPoller::EPoller(EventLoop *loop)
    : ownerLoop_(loop), epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOGFATAL << "EPoller::EPoller";
  }
}

EPoller::~EPoller() { ::close(epollfd_); }

long EPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);

  timer::Timestamp now =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count();

  if (numEvents > 0) {
    LOGTARCE << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
    if (static_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOGTARCE << "nothing happended";
  } else {
    LOGFATAL << "EPoller::poll()";
  }
  return now;
}

void EPoller::fillActiveChannels(int numEvents,
                                 ChannelList *activeChannels) const {
  assert(static_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    int fd = channel->fd();
    const auto &it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void EPoller::updateChannel(Channel *channel) {
  LOGTARCE << "fd = " << channel->fd() << " events = " << channel->events();
  const int index = channel->index();
  if (index == kNew || index == kDeleted) {
    int fd = channel->fd();
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPoller::removeChannel(Channel *channel) {
  int fd = channel->fd();
  LOGTARCE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }

  channel->set_index(kNew);
}

void EPoller::update(int operation, Channel *channel) {
  struct epoll_event event;

  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOGERROR << "epoll_ctl op=" << operation << " fd=" << fd;
    } else {
      LOGFATAL << "epoll_ctl op=" << operation << " fd=" << fd;
    }
  }
}

} // namespace reactor
} // namespace znet
