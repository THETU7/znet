#include "Poller.h"

#include <algorithm>
#include <assert.h>
#include <poll.h>

#include <chrono>

#include "Channel.h"

namespace znet {
namespace reactor {
Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

long Poller::poll(int timeoutMs, ChannelList *activeChannels) {
  assertInLoopThread();
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
  long now = std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  if (numEvents > 0) {
    LOGTARCE << numEvents << " events happend";
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    LOGTARCE << "nothing happend";
  } else {
    LOGERROR << "Poller::poll";
  }
  return now;
}

void Poller::fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const {
  for (auto it = pollfds_.begin(); it != pollfds_.end() && numEvents > 0;
       ++it) {
    if (it->revents > 0) {
      LOGTARCE << "activeFd " << it->fd;
      --numEvents;
      auto ch = channels_.find(it->fd);
      assert(ch != channels_.end());
      Channel *channel = ch->second;
      assert(channel->fd() == it->fd);
      channel->set_revents(it->revents);
      activeChannels->push_back(channel);
    }
  }
}

void Poller::updateChannel(Channel *ch) {
  assertInLoopThread();
  LOGTARCE << "Update Channel fd = " << ch->fd()
           << " events = " << ch->events();

  if (ch->index() < 0) {
    assert(channels_.find(ch->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = ch->fd();
    pfd.events = static_cast<short>(ch->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size() - 1);
    ch->setIndex(idx);
    channels_[ch->fd()] = ch;
  } else {
    assert(channels_.find(ch->fd()) != channels_.end());
    assert(channels_[ch->fd()] == ch);

    int idx = ch->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd &pfd = pollfds_[idx];
    assert(pfd.fd == ch->fd() || pfd.fd == -ch->fd() - 1);
    pfd.events = static_cast<short>(ch->events());
    pfd.revents = 0;
    if (ch->isNoneEvent()) {
      pfd.fd = -ch->fd() - 1;
    }
  }
}

void Poller::removeChannel(Channel *channel) {
  assertInLoopThread();
  LOGTARCE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd &pfd = pollfds_[idx];
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1);
  if (static_cast<size_t>(idx) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  } else {
    int channelAtEnd = pollfds_.back().fd;
    std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd - 1;
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}

} // namespace reactor
} // namespace znet
