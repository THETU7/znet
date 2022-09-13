/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-08                                                   #
  # @ File: src/TcpServer.h                                              #
  #                                                                      #
  # @ Description: Tcp Server                                            #
  ########################################################################*/

#ifndef REACTOR_SRC_TCPSERVER_H_
#define REACTOR_SRC_TCPSERVER_H_

#include <map>
#include <memory>

#include <boost/core/noncopyable.hpp>

#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"

namespace znet {
class TcpServer : public boost::noncopyable {
public:
  TcpServer(reactor::EventLoop *loop, const Inetaddress &listenAddr);
  ~TcpServer();

  void setThreadNums(int nums) noexcept { threadPool_.setThreadNums(nums); }
  void start();
  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

private:
  void newConnection(Socket &&sockfd, const Inetaddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &);
  // no thread safe but in loop
  void removeConnectionInLoop(const TcpConnectionPtr &);

  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

  reactor::EventLoop *loop_;
  const std::string name_;
  Acceptor acceptor_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool started;
  int nextConnId_;
  ConnectionMap connections_;
  EventLoopThreadPool threadPool_;
};
} // namespace znet

#endif // !REACTOR_SRC_TCPSERVER_H_
