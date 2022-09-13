/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-08                                                   #
  # @ File: src/Socket.h                                                 #
  #                                                                      #
  # @ Description: A RAII Class of socketfd                              #
  ########################################################################*/

#ifndef REACTOR_SRC_SOCKET_H_
#define REACTOR_SRC_SOCKET_H_

#include <unistd.h>

#include <boost/core/noncopyable.hpp>

#include "InetAddress.h"

namespace znet {
class Socket : boost::noncopyable {
public:
  explicit Socket();
  ~Socket() {
    if (socketFd_ >= 0) {
      ::close(socketFd_);
    }
  }
  explicit Socket(int sockfd) : socketFd_(sockfd) {}
  Socket(Socket &&rhs) {
    socketFd_ = rhs.socketFd_;
    rhs.socketFd_ = -1;
  }

  int fd() const { return socketFd_; }
  void bindAddress(const Inetaddress &localaddr);
  void listen();

  int accpect(Inetaddress *peeraddr);
  void setReuseAddr(bool on);
  void shutdownWrite();
  void setTcpNoDelay(bool on);
  void setKeepAlive(bool on);

private:
  int socketFd_;
};
} // namespace znet

#endif // !REACTOR_SRC_SOCKET_H_
