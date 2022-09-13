#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"
#include "TcpConnection.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

std::string message = "Hello\n";

void onConnection(const znet::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    conn->send(message);
    conn->shutdown();

  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const znet::TcpConnectionPtr &conn, znet::buffer::Buffer *buf,
               znet::timer::Timestamp receiveTime) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readAbleBytes(), conn->name().c_str(),
         znet::timer::formatTimestamp(receiveTime).c_str());

  printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

void onWriteCompleteCallBack(const znet::TcpConnectionPtr &conn) {
  auto loop = conn->getLoop();
  loop->quit();
}

int main() {
  znet::reactor::EventLoop loop;
  znet::Inetaddress serverAddr("localhost", 9981);
  znet::TcpClient client(&loop, serverAddr);

  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
  client.setWriteCompleteCallBack(onWriteCompleteCallBack);
  client.enableRetry();
  client.connect();
  loop.loop();
}
