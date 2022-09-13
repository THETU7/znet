#include "CurrentThread.h"
#include "EventLoopThread.h"
#include "LogStream.h"
#include <stdio.h>
#include <thread>

void runInThread() {
  printf("runInThread(): pid = %d, tid = %d\n", getpid(),
         znet::CurrentThread::tid());
}

int main(int argc, char *argv[]) {
  znet::EvenetThread et;
  znet::reactor::EventLoop *loop = et.startLoop();
  loop->runInloop(runInThread);
  sleep(1);
  loop->runAfter(2, runInThread);
  sleep(3);
  loop->quit();
  printf("exit main().\n");

  return 0;
}
