#include "Coonector.h"

#include "SocketsOps.h"
#include <sys/socket.h>

namespace znet {
const int Connector::kMaxRetryDelayMs;

Connector::Connector(reactor::EventLoop *loop, const Inetaddress &serveraddr)
    : loop_(loop), serveraddr_(serveraddr), connect_(false),
      state_(States::kDisconnected), retryDelayMs_(kInitRetryDelayMs) {}

Connector::~Connector() {
  LOGDEBUG << "dtor[" << this << "]";
  loop_->cancel(timerId_);
  assert(!channel_);
}

void Connector::start() {
  connect_ = true;
  loop_->runInloop([ptr = this]() { ptr->startInLoop(); });
}

void Connector::startInLoop() {
  loop_->assertInLoopThread();
  assert(state_ == States::kDisconnected);
  if (connect_) {
    connect();
  } else {
    LOGDEBUG << "do not connect";
  }
}

void Connector::connect() {
  int sockfd = sockets::createNonblockingOrDie();
  int ret = sockets::connect(sockfd, serveraddr_.getSockAddrInet());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
  case 0:
  case EINPROGRESS:
  case EINTR:
  case EISCONN:
    connecting(sockfd);
    break;

  case EAGAIN:
  case EADDRINUSE:
  case EADDRNOTAVAIL:
  case ECONNREFUSED:
  case ENETUNREACH:
    retry(sockfd);
    break;

  case EACCES:
  case EPERM:
  case EAFNOSUPPORT:
  case EALREADY:
  case EBADF:
  case EFAULT:
  case ENOTSOCK:
    LOGERROR << "connect error in Connector::startInLoop " << savedErrno;
    sockets::close(sockfd);
    break;
  default:
    LOGERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
    sockets::close(sockfd);
    break;
  }
}

void Connector::restart() {
  loop_->assertInLoopThread();
  setState(States::kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

void Connector::stop() {
  connect_ = false;
  // loop_->cannel(timerID)
}

void Connector::connecting(int sockfd) {
  setState(States::kConnecting);
  assert(!channel_);
  channel_.reset(new reactor::Channel(loop_, sockfd));
  channel_->setWriteCallBack([ptr = this]() { ptr->handleWrite(); });
  channel_->setErrorCallBack([ptr = this]() { ptr->handleError(); });
  channel_->enableWrite();
}

int Connector::removeAndResetChannel() {
  channel_->disableAll();
  loop_->removeChannel(channel_.get());
  int sockfd = channel_->fd();
  loop_->queueInLoop([ptr = this]() { ptr->resetChannel(); });
  return sockfd;
}

void Connector::resetChannel() { channel_.reset(); }

void Connector::handleWrite() {
  LOGTARCE << "Connector::handleWrite";
  if (state_ == States::kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err) {
      LOGWARN << "Connector::handleWrite - SO_ERROR = " << err << " "
              << strerror(err);
      retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
      LOGWARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    } else {
      setState(States::kConnected);
      if (connect_) {
        newConnectionCallback_(Socket(sockfd), serveraddr_);
      } else {
        sockets::close(sockfd);
      }
    }
  } else {
    assert(state_ == States::kDisconnected);
  }
}

void Connector::handleError() {
  LOGERROR << "Connector::handleError";
  assert(state_ == States::kConnecting);

  int sockfd = removeAndResetChannel();
  int err = sockets::getSocketError(sockfd);
  LOGTARCE << "SO_ERROR = " << err << " " << strerror(err);
  retry(sockfd);
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(States::kDisconnected);
  if (connect_) {
    LOGINFO << "Connector::retry - Retry connecting to "
            << serveraddr_.toHostPort() << " in " << retryDelayMs_
            << " milliseconds. ";
    timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0, // FIXME: unsafe
                               [ptr = this]() { ptr->startInLoop(); });
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  } else {
    LOGDEBUG << "do not connect";
  }
}

} // namespace znet
