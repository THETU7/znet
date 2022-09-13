#include "Buffer.h"
#include "Callbacks.h"
#include "CurrentThread.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "TcpServer.h"

void onConnectBack(const znet::TcpConnectionPtr &coon) {
  if (coon->connected()) {
    printf("onConnection(): new connection [%s] form %s in thread %d\n",
           coon->name().c_str(), coon->peerAddress().toHostPort().c_str(),
           znet::CurrentThread::tid());
  } else {
    printf("onConnection(): connection [%s] is down\n", coon->name().c_str());
  }
}

void onMessage(const znet::TcpConnectionPtr &coon, znet::buffer::Buffer *buffer,
               znet::timer::Timestamp time) {
  printf("onMessage(): received %zd bytes from connection[%s] at %s in thread "
         "%d\n",
         buffer->readAbleBytes(), coon->name().c_str(),
         znet::timer::formatTimestamp(time).c_str(),
         znet::CurrentThread::tid());

  printf("Message: %s\n", buffer->retrieveAsString().c_str());
}

int main(int argc, char *argv[]) {
  printf("main(): pid = %d tid = %d\n", getpid(), znet::CurrentThread::tid());
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  znet::reactor::EventLoop loop;
  znet::Inetaddress addr(9981);
  znet::TcpServer server(&loop, addr);
  server.setThreadNums(2);
  server.setConnectionCallback(onConnectBack);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
  return 0;
}
