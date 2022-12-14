#include "EventLoop.h"
#include "LogStream.h"
#include <stdio.h>

znet::reactor::EventLoop *g_loop;
int g_flag = 0;

void run4() {
  printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->quit();
}

void run3() {
  printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->runAfter(3, run4);
  g_flag = 3;
}

void run2() {
  printf("run2(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->queueInLoop(run3);
}

void run1() {
  g_flag = 1;
  printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->runInloop(run2);
  g_flag = 2;
}

int main() {
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);

  znet::reactor::EventLoop loop;
  g_loop = &loop;

  loop.runAfter(2, run1);
  loop.loop();
  printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);
}
