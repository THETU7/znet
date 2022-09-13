#include "TcpClient.h"
#include "TcpConnection.h"

znet::reactor::EventLoop *gloop;

void onMessage(const znet::TcpConnectionPtr &ptr, znet::buffer::Buffer *buffer,
               znet::timer::Timestamp now) {
  LOGINFO << buffer->retrieveAsString();
  ptr->shutdown();
  gloop->quit();
}

int main(int argc, char *argv[]) {
  znet::Inetaddress addr("localhost", 9981);
  znet::reactor::EventLoop loop;
  gloop = &loop;
  znet::TcpClient client(&loop, addr);
  client.setMessageCallback(onMessage);
  client.enableRetry();
  client.connect();
  loop.loop();
  return 0;
}
