#include "TcpServer.h"

std::string message1;
std::string message2;

void onConnection(const znet::TcpConnectionPtr &coon) {
  if (coon->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           coon->name().c_str(), coon->peerAddress().toHostPort().c_str());
  } else {
    printf("onConnection(): connection [%s] is down\n", coon->name().c_str());
  }

  ::sleep(5);
  coon->send("test");
  coon->send("message");
}

void onMessage(const znet::TcpConnectionPtr &conn, znet::buffer::Buffer *buffer,
               znet::timer::Timestamp time) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buffer->readAbleBytes(), conn->name().c_str(),
         znet::timer::formatTimestamp(time).c_str());

  conn->send(buffer->retrieveAsString());
}

int main(int argc, char *argv[]) {
  AsyncLog::LogStream::setLogLevel(AsyncLog::LogLevel::TARCE);
  znet::reactor::EventLoop loop;
  znet::Inetaddress addr(9981);
  znet::TcpServer server(&loop, addr);
  server.setThreadNums(0);

  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
  return 0;
}
