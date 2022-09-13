#include "TcpConnection.h"
#include "TcpServer.h"

void onMessage(const znet::TcpConnectionPtr &conn, znet::buffer::Buffer *buffer,
               znet::timer::Timestamp time) {
  conn->send(buffer->retrieveAsString());
}

int main(int argc, char *argv[]) {
  znet::EventLoop loop;
  znet::Inetaddress addr("127.0.0.1", 9981);
  znet::TcpServer server(&loop, addr);
  server.setMessageCallback(onMessage);
  server.setThreadNums(std::thread::hardware_concurrency());
  server.start();
  loop.loop();
  return 0;
}
