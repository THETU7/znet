#include "Acceptor.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Socket.h"

namespace znet {
Acceptor::Acceptor(reactor::EventLoop *loop, const Inetaddress &addr)
    : loop_(loop), socket_(), accpectChannel_(loop, socket_.fd()),
      listenning_(false) {
  socket_.setReuseAddr(true);
  socket_.bindAddress(addr);
  accpectChannel_.setReadCallBack(
      [ptr = this](timer::Timestamp) { ptr->handleReader(); });
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listenning_ = true;
  socket_.listen();
  accpectChannel_.enableReading();
}

void Acceptor::handleReader() {
  loop_->assertInLoopThread();
  Inetaddress peerAddr(0);
  int coond = socket_.accpect(&peerAddr);
  Socket coon(coond);
  if (coond > 0) {
    if (newConnectCallbcak_) {
      newConnectCallbcak_(std::move(coon), peerAddr);
    }
  }
}

} // namespace znet
