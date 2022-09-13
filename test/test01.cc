#include "CurrentThread.h"
#include "EventLoop.h"
#include "LogStream.h"
#include "Logging.h"

#include <thread>

void threadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(),
         znet::CurrentThread::tid());

  znet::reactor::EventLoop loop;
  loop.loop();
}

int main(int argc, char *argv[]) {
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  printf("main(): pid = %d, tid = %d\n", getpid(), znet::CurrentThread::tid());
  znet::reactor::EventLoop eloop;

  std::thread t1(threadFunc);

  eloop.loop();
  t1.join();
  return 0;
}
