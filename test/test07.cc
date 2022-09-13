#include "Acceptor.h"
#include "EventLoopThread.h"
#include "LogStream.h"
#include <ctime>
#include <iomanip>

void newConnect(znet::Socket &&sockfd, const znet::Inetaddress &peerAddr) {
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());

  ::write(sockfd.fd(), "How Are you\n", 13);
  ::close(sockfd.fd());
}

void dayTime(znet::Socket &&sockfd, const znet::Inetaddress &peerAddr) {
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  char *re = ctime(&now);
  ::write(sockfd.fd(), re, strlen(re));
}

int main(int argc, char *argv[]) {
  // znet::EvenetThread loopThread;
  // znet::reactor::EventLoop *loop = loopThread.startLoop();
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  znet::reactor::EventLoop loop;

  znet::Inetaddress addr(9999);
  znet::Inetaddress addr2(19999);
  znet::Acceptor acceptor(&loop, addr);
  znet::Acceptor acceptor2(&loop, addr2);
  acceptor.setNewConnectCallback(dayTime);
  acceptor2.setNewConnectCallback(newConnect);

  acceptor.listen();
  acceptor2.listen();
  loop.loop();

  return 0;
}
