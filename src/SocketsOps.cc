#include "SocketsOps.h"

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>

#include <boost/implicit_cast.hpp>

#include "AsyncLog.h"

namespace znet {
namespace sockets {

const SA *sockaddr_cast(const struct sockaddr_in *addr) {
  return static_cast<const SA *>(static_cast<const void *>(addr));
}

SA *sockaddr_cast(struct sockaddr_in *addr) {
  return static_cast<SA *>(boost::implicit_cast<void *>(addr));
}

void setNonBlockAndCloseOnExec(int sockfd) {
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check
}

void fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) < 0) {
    LOGFATAL << "sockets::fromHostPort";
  }
}

void toHostPort(char *buf, size_t size, const struct sockaddr_in *addr) {
  char host[INET_ADDRSTRLEN] = "INVAILD";
  ::inet_ntop(AF_INET, &addr->sin_addr, host, sizeof host);
  uint16_t port = sockets::networkToHost16(addr->sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void bindOrDie(int socket, const struct sockaddr_in &addr) {
  int ret = ::bind(socket, sockaddr_cast(&addr), sizeof addr);
  if (ret < 0) {
    LOGFATAL << "sockets::bindOrDie";
  }
}

void listeningOrDie(int socket) {
  int ret = ::listen(socket, SOMAXCONN);
  LOGTARCE << "listen " << ret;
  if (ret < 0) {
    LOGFATAL << "sockets::listeningOrDie";
  }
}

int accept(int sockfd, struct sockaddr_in *addr) {
  socklen_t addrlen = sizeof *addr;
#if VALGRIND
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0) {
    int savedErrno = errno;
    LOGERROR << "Socket::accept";
    switch (savedErrno) {
    case EAGAIN:
    case ECONNABORTED:
    case EINTR:
    case EPROTO: // ???
    case EPERM:
    case EMFILE: // per-process lmit of open file desctiptor ???
      // expected errors
      errno = savedErrno;
      break;
    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
    case ENOTSOCK:
    case EOPNOTSUPP:
      // unexpected errors
      LOGFATAL << "unexpected error of ::accept " << savedErrno;
      break;
    default:
      LOGFATAL << "unknown error of ::accept " << savedErrno;
      break;
    }
  }
  return connfd;
}

int createNonblockingOrDie() {
#if VALGRIND
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
  }

  setNonBlockAndCloseOnExec(sockfd);
#else
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);
  if (sockfd < 0) {
    LOGFATAL << "sockets::createNonblockingOrDie";
  }
#endif
  return sockfd;
}

struct sockaddr_in getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    LOGFATAL << "sockets::getLocalAddr";
  }
  return localaddr;
}

int getSocketError(int sockfd) noexcept {
  int optval;
  socklen_t optlen = sizeof optval;
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

void shutDownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOGERROR << "sockets::shutDownWrite";
  }
}

const SA *sockaddr_cast(const struct sockaddr_in &addr) {
  return static_cast<const SA *>(static_cast<const void *>(&addr));
}

int connect(int fd, const struct sockaddr_in &addr) {
  return ::connect(fd, sockaddr_cast(addr), sizeof addr);
}

void close(int fd) {
  if (::close(fd) < 0) {
    LOGERROR << "sockets::close";
  }
}

struct sockaddr_in getPeerAddr(int sockfd) {
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = sizeof(peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
    LOGERROR << "sockets::getPeerAddr";
  }
  return peeraddr;
}

bool isSelfConnect(int sockfd) {
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port &&
         localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

} // namespace sockets
} // namespace znet
