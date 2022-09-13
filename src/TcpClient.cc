#include "TcpClient.h"

#include <mutex>
#include <netinet/in.h>

#include "Socket.h"
#include "SocketsOps.h"
#include "TcpConnection.h"

namespace znet {

void removeConnection(reactor::EventLoop *loop, const TcpConnectionPtr &conn) {
  loop->runInloop([ptr = conn]() { ptr->connectDestroyed(); });
}

TcpClient::TcpClient(reactor::EventLoop *loop, const Inetaddress &addr)
    : loop_(loop), name_(addr.toHostPort()), connector_(loop, addr),
      nextConnId_(1), serverAddr_(addr), started(false) {
  connector_.setNewConnectionCallback(
      [ptr = this](Socket &&socket, const Inetaddress &addr) {
        ptr->newConnection(std::move(socket), addr);
      });
  LOGINFO << "TcpClient::TcpClient[" << this << "] - connector " << &connector_;
}

TcpClient::~TcpClient() {
  LOGINFO << "TcpClient::~TcpClient[" << this << "] - connector "
          << &connector_;
  bool unique = false;
  TcpConnectionPtr conn;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    unique = tcpConnection_.unique();
    conn = tcpConnection_;
  }
  if (conn) {
    assert(loop_ == conn->getLoop());
    CloseCallBack cb =
        std::bind(&znet::removeConnection, loop_, std::placeholders::_1);
    loop_->runInloop(std::bind(&TcpConnection::setCloseCallBack, conn, cb));
    if (unique) {
      conn->forceClose();
    }
  } else {
    connector_.stop();
  }
}

TcpConnectionPtr TcpClient::connection() {
  std::lock_guard<std::mutex> lock(mutex_);
  return tcpConnection_;
}

void TcpClient::connect() {
  if (!started) {
    connector_.start();
    started = true;
  }
}

void TcpClient::disConnect() {
  started = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (tcpConnection_) {
      tcpConnection_->shutdown();
    }
  }
}

void TcpClient::stop() {
  started = false;
  connector_.stop();
}

void TcpClient::newConnection(Socket &&socket, const Inetaddress &addr) {
  Inetaddress localAddr(sockets::getLocalAddr(socket.fd()));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", serverAddr_.toHostPort().c_str(),
           nextConnId_);
  nextConnId_++;
  std::string conname = name_ + buf;

  LOGINFO << "TcpClient::newConnection [" << name_ << "] to "
          << serverAddr_.toHostPort();

  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      loop_, conname, std::move(socket), localAddr, serverAddr_);
  if (connectionCallback_)
    conn->setConnectionCallback(connectionCallback_);
  if (messageCallback_)
    conn->setMessageCallback(messageCallback_);
  if (writeCompleteCallback_)
    conn->setWriteCompleteCallBack(writeCompleteCallback_);
  if (closeCallBack_)
    conn->setCloseCallBack(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    tcpConnection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(tcpConnection_ == conn);
    tcpConnection_.reset();
  }
  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && tcpConnection_) {
    LOGINFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
            << serverAddr_.toHostPort();
    connector_.restart();
  }
}

void TcpClient::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  LOGINFO << "TcpClient::removeConnection [" << name_ << "]";
  loop_->queueInLoop([ptr = conn]() { ptr->connectDestroyed(); });
  tcpConnection_ = nullptr;
}

} // namespace znet
