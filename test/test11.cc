#include "CurrentThread.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpServer.h"
#include <stdio.h>

std::string message;

void onConnection(const znet::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s in thread %d\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str(),
           znet::CurrentThread::tid());
    conn->send(message);
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onWriteComplete(const znet::TcpConnectionPtr &conn) {
  printf("onWriteComplete()\n");
  conn->send(message);
}

void onMessage(const znet::TcpConnectionPtr &conn, znet::buffer::Buffer *buf,
               znet::timer::Timestamp receiveTime) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s in thread "
         "%d\n",
         buf->readAbleBytes(), conn->name().c_str(),
         znet::timer::formatTimestamp(receiveTime).c_str(),
         znet::CurrentThread::tid());

  conn->send(buf->retrieveAsString());
}

int main() {
  printf("main(): pid = %d\n", getpid());

  std::string line;
  for (int i = 33; i < 127; ++i) {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127 - 33; ++i) {
    message += line.substr(i, 72) + '\n';
  }

  znet::Inetaddress listenAddr(9981);
  znet::reactor::EventLoop loop;

  znet::TcpServer server(&loop, listenAddr);
  server.setThreadNums(3);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.setWriteCompleteCallback(onWriteComplete);
  server.start();

  loop.loop();
}
