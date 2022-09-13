#include "Socket.h"
#include "SocketsOps.h"

#include <netinet/tcp.h>

#include <strings.h>

namespace znet {

Socket::Socket() : socketFd_(sockets::createNonblockingOrDie()) {}

void Socket::bindAddress(const Inetaddress &localaddr) {
  sockets::bindOrDie(socketFd_, localaddr.getSockAddrInet());
}

void Socket::listen() { sockets::listeningOrDie(socketFd_); }

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

int Socket::accpect(Inetaddress *peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(socketFd_, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddrInet(addr);
  }
  return connfd;
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketFd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::shutdownWrite() { sockets::shutDownWrite(socketFd_); }

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketFd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               static_cast<socklen_t>(sizeof optval));
}

} // namespace znet
