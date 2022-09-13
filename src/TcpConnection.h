/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-08                                                   #
  # @ File: src/TcpConnection.h                                          #
  #                                                                      #
  # @ Description: handle the connection from Accpector                  #
  ########################################################################*/

#ifndef REACTOR_SRC_TCPCONNECTION_H_
#define REACTOR_SRC_TCPCONNECTION_H_

#include <boost/core/noncopyable.hpp>

#include "Buffer.h"
#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

namespace znet {
class TcpConnection : public boost::noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(reactor::EventLoop *loop, const std::string name,
                Socket &&socket, const Inetaddress &localAddr,
                const Inetaddress &peerAddr);

  ~TcpConnection();

  reactor::EventLoop *getLoop() { return loop_; }
  const Inetaddress &localAddress() { return localAddr_; }
  const Inetaddress &peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == StateE::kConnected; }
  const std::string &name() const { return name_; }

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  void setCloseCallBack(const CloseCallBack &cb) { closeCallBack_ = cb; };
  void setWriteCompleteCallBack(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }
  /// Internal use only.

  // called when TcpServer accepts a new connection
  void connectEstablished(); // should be called only onc
                             //
  void connectDestroyed();

  // void send(const void *message, size_t len);
  void send(const std::string &message);
  void shutdown();

  void setTcpNoDelay(bool on);
  void setKeepAlive(bool on);

  void forceClose();
  void forceCloseInLoop();

private:
  enum class StateE { KConnecting, kConnected, kDisconnecting, kDisconnected };

  void handleRead(timer::Timestamp time);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const std::string &message);
  void shutdownInLoop();
  void setState(StateE s) { state_ = s; }

  reactor::EventLoop *loop_;
  std::string name_;
  StateE state_;
  Socket socket_;
  reactor::Channel channel_;
  Inetaddress localAddr_;
  Inetaddress peerAddr_;
  buffer::Buffer inputBuffer_;
  buffer::Buffer outputBuffer_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallBack closeCallBack_;
  WriteCompleteCallback writeCompleteCallback_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace znet

#endif // !REACTOR_SRC_TCPCONNECTION_H_
