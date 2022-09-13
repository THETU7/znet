#include "TcpServer.h"

#include "SocketsOps.h"

namespace znet {
TcpServer::TcpServer(reactor::EventLoop *loop, const Inetaddress &listenAddr)
    : loop_(loop), name_(listenAddr.toHostPort()), acceptor_(loop_, listenAddr),
      started(false), nextConnId_(1), threadPool_(loop) {
  acceptor_.setNewConnectCallback(
      [ptr = this](Socket &&socket, const Inetaddress &peerAddr) {
        ptr->newConnection(std::move(socket), peerAddr);
      });
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
  if (!started) {
    acceptor_.listen();
    threadPool_.start();
    started = true;
  }

  if (!acceptor_.listenning()) {
    loop_->runInloop(
        [acceptor_ = std::ref(acceptor_)]() { acceptor_.get().listen(); });
  }
}

void TcpServer::newConnection(Socket &&sockfd, const Inetaddress &peerAddr) {
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof buf, "#%d", nextConnId_);
  ++nextConnId_;
  std::string coonName = name_ + buf;

  LOGINFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
          << coonName << "] from " << peerAddr.toHostPort();

  Inetaddress localAddr(sockets::getLocalAddr(sockfd.fd()));

  EventLoop *ioLoop = threadPool_.getNextLoop();

  TcpConnectionPtr coon = std::make_shared<TcpConnection>(
      ioLoop, coonName, std::move(sockfd), localAddr, peerAddr);

  connections_[coonName] = coon;
  coon->setConnectionCallback(connectionCallback_);
  coon->setMessageCallback(messageCallback_);
  coon->setWriteCompleteCallBack(writeCompleteCallback_);
  coon->setCloseCallBack([ptr = this](const TcpConnectionPtr &coon) {
    ptr->removeConnection(coon);
  });
  ioLoop->runInloop([coon = coon]() { coon->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &coon) {
  loop_->runInloop(
      [ptr = this, coon = coon]() { ptr->removeConnectionInLoop(coon); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &coon) {
  loop_->assertInLoopThread();
  LOGINFO << "TcpServer::removeConnection [" << name_ << "] - connection "
          << coon->name();
  size_t n = connections_.erase(coon->name());
  assert(n == 1);
  EventLoop *ioLoop = coon->getLoop();
  // 如果这里传输引用，由于 TcoServer 已经 erase ，那么引用计数就会为0,导致析构
  ioLoop->queueInLoop([ptr = coon]() { ptr->connectDestroyed(); });
}

} // namespace znet
