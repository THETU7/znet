# ZNET

A Tcp library that support server and client development

## Install

```bash
mkdir build && cd build
cmake ..
make -j4
make install
```

## Usage

- server

```cpp
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

```

- client

```cpp
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
```

## Reference

- [muduo](https://github.com/chenshuo/muduo)
