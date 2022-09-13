#include "Channel.h"
#include "EventLoop.h"
#include "LogStream.h"
#include "TimerQueue.h"

int main(int argc, char *argv[]) {
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  znet::reactor::EventLoop loop;
  loop.runAfter(1, []() { printf("runAfter 1s\n"); });
  loop.runAt(std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
                     .count() +
                 2,
             []() { printf("runAt\n"); });
  loop.runEvery(1.3, []() { printf("run every 1.3s\n"); });
  loop.runAfter(4, [ptr = &loop]() { ptr->quit(); });
  loop.loop();
  return 0;
}
