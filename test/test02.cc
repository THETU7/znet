#include "EventLoop.h"

#include <thread>

znet::reactor::EventLoop *g_loop;

void threadFunc() { g_loop->loop(); }

int main() {
  znet::reactor::EventLoop loop;
  g_loop = &loop;
  std::thread t(threadFunc);
  t.join();
}
