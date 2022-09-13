#include "TcpClient.h"
#include "TcpConnection.h"
#include <cstring>

char *data;

void onConnection(const znet::TcpConnectionPtr &conn) {
  LOGINFO << data;
  conn->send(data);
}

void onMessage(const znet::TcpConnectionPtr &conn, znet::buffer::Buffer *buffer,
               znet::timer::Timestamp now) {
  // LOGINFO << buffer->retrieveAsString();
  conn->send(buffer->retrieveAsString());
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    LOGFATAL << "usage: pingpongClient [nums]";
  }
  char buf[atoi(argv[1]) + 1];
  memset(buf, 'a', atoi(argv[1]));
  buf[atoi(argv[1])] = '\0';
  data = buf;

  znet::reactor::EventLoop loop;
  znet::Inetaddress addr("127.0.0.1", 9981);
  znet::TcpClient client(&loop, addr);
  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
  client.connect();
  loop.loop();
  return 0;
}
