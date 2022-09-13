#include "Coonector.h"
#include "EventLoop.h"

znet::reactor::EventLoop *gloop;

void connectCallback(znet::Socket &&socket, const znet::Inetaddress &addr) {
  printf("connected.\n");
  gloop->quit();
}

int main(int argc, char *argv[]) {
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  znet::reactor::EventLoop loop;
  gloop = &loop;
  znet::Inetaddress addr(9981);
  znet::Connector conn(&loop, addr);
  conn.setNewConnectionCallback(connectCallback);
  conn.start();
  loop.loop();
  return 0;
}
