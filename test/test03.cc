#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"

#include <sys/timerfd.h>

znet::reactor::EventLoop *g_loop;

void timeout(znet::timer::Timestamp time) {
  printf("%ld Timeout!\n", time);
  g_loop->quit();
}

int main(int argc, char *argv[]) {
  znet::reactor::EventLoop loop;
  g_loop = &loop;
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

  znet::reactor::Channel channel(&loop, timerfd);
  channel.setReadCallBack(timeout);
  channel.enableReading();

  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();

  ::close(timerfd);

  return 0;
}
