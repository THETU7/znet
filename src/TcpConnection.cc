#include "TcpConnection.h"
#include "Callbacks.h"
#include "SocketsOps.h"

#include <assert.h>

namespace znet {
TcpConnection::TcpConnection(reactor::EventLoop *loop, const std::string name,
                             Socket &&socket, const Inetaddress &localAddr,
                             const Inetaddress &peerAddr)
    : loop_(loop), name_(name), state_(StateE::KConnecting),
      socket_(std::move(socket)), channel_(loop, socket_.fd()),
      localAddr_(localAddr), peerAddr_(peerAddr) {
  LOGDEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
           << " fd=" << socket.fd();
  channel_.setReadCallBack(
      [ptr = this](timer::Timestamp time) { ptr->handleRead(time); });
  channel_.setCloseCallBack([ptr = this]() { ptr->handleClose(); });
  channel_.setWriteCallBack([ptr = this]() { ptr->handleWrite(); });
  channel_.setErrorCallBack([ptr = this]() { ptr->handleError(); });
}

TcpConnection::~TcpConnection() {
  LOGDEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
           << " fd=" << channel_.fd();
}

void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == StateE::KConnecting);
  setState(StateE::kConnected);
  channel_.enableReading();

  if (connectionCallback_)
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead(timer::Timestamp time) {
  int saveError = 0;
  ssize_t n = inputBuffer_.readFd(socket_.fd(), &saveError);
  if (n > 0) {
    if (messageCallback_)
      messageCallback_(shared_from_this(), &inputBuffer_, time);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = saveError;
    LOGERROR << "TcpConnection::handleRead()";
    handleError();
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOGTARCE << "TcpConnection::handleClose() state = "
           << static_cast<int>(state_);
  assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
  // 将 channel 设为 kNone;
  channel_.disableAll();
  if (closeCallBack_)
    closeCallBack_(shared_from_this());
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(socket_.fd());
  LOGERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR" << err
           << " " << strerror(err);
}

void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
  setState(StateE::kDisconnected);
  channel_.disableAll();
  if (connectionCallback_)
    connectionCallback_(shared_from_this());
  loop_->removeChannel(&channel_);
}

void TcpConnection::shutdown() {
  if (state_ == StateE::kConnected) {
    setState(StateE::kDisconnecting);
    loop_->runInloop([ptr = this]() { ptr->shutdownInLoop(); });
  }
}

void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_.isWriting()) {
    socket_.shutdownWrite();
  }
}

void TcpConnection::send(const std::string &message) {
  if (state_ == StateE::kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      // 这里传引用可能会导致生命周期问题
      loop_->runInloop(
          [ptr = this, message = message]() { ptr->sendInLoop(message); });
    }
  }
}

void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_.isWriting()) {
    ssize_t n = ::write(channel_.fd(), outputBuffer_.peek(),
                        outputBuffer_.readAbleBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readAbleBytes() == 0) {
        channel_.disableWrite();
        if (writeCompleteCallback_) {
          loop_->queueInLoop([ptr = shared_from_this(),
                              cb = writeCompleteCallback_]() { cb(ptr); });
        }
        if (state_ == StateE::kDisconnecting) {
          shutdownInLoop();
        }
      } else {
        LOGTARCE << "i am going to write more data";
      }
    } else {
      LOGERROR << "TcpConnection::handleWrite";
    }
  } else {
    LOGTARCE << "Connection is down, no more writing";
  }
}

void TcpConnection::sendInLoop(const std::string &message) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  if (!channel_.isWriting() && outputBuffer_.readAbleBytes() == 0) {
    nwrote = ::write(channel_.fd(), message.data(), message.size());
    if (nwrote >= 0) {
      if (static_cast<size_t>(nwrote) < message.size()) {
        LOGTARCE << "I am going to write more data";
      } else if (writeCompleteCallback_) {
        loop_->queueInLoop([ptr = shared_from_this(),
                            cb = writeCompleteCallback_]() { cb(ptr); });
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOGERROR << "TcpConnection::sentInLoop";
      }
    }
  }
  assert(nwrote >= 0);

  if (static_cast<size_t>(nwrote) < message.size()) {
    outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
    if (!channel_.isWriting()) {
      channel_.enableWrite();
    }
  }
}

void TcpConnection::setTcpNoDelay(bool on) { socket_.setTcpNoDelay(on); }
void TcpConnection::setKeepAlive(bool on) { socket_.setKeepAlive(true); }

void TcpConnection::forceClose() {
  // FIXME: use compare and swap
  if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
    setState(StateE::kDisconnecting);
    loop_->queueInLoop(
        std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}

} // namespace znet
