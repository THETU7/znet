#include "TcpServer.h"

int main(int argc, char *argv[]) {
  znet::Inetaddress addr("localhost", 9981);
  znet::EventLoop loop;
  znet::TcpServer server(&loop, addr);
  server.setThreadNums(std::thread::hardware_concurrency());
  server.setConnectionCallback([](const znet::TcpConnectionPtr &ptr) {
    znet::timer::Timestamp now =
        std::chrono::system_clock::now().time_since_epoch().count();
    ptr->send(znet::timer::formatTimestamp(now));
  });
  server.start();
  loop.loop();
  return 0;
}
