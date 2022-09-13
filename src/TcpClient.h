/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-10                                                   #
  # @ File: src/TcpClient.h                                              #
  #                                                                      #
  # @ Description: handle send and read msg form server                  #
  ########################################################################*/

#ifndef REACTOR_SRC_TCPCLIENT_H_
#define REACTOR_SRC_TCPCLIENT_H_

#include <boost/core/noncopyable.hpp>
#include <mutex>

#include "Callbacks.h"
#include "Coonector.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Socket.h"

namespace znet {
class TcpClient : boost::noncopyable {
public:
  TcpClient(reactor::EventLoop *loop, const Inetaddress &addr);
  ~TcpClient();

  void connect();
  void disConnect();
  void reStart();
  void stop();
  TcpConnectionPtr connection();
  reactor::EventLoop *getLoop() { return loop_; }
  void enableRetry() { retry_ = true; }
  bool retry() { return retry_; }
  const std::string &name() { return name_; }
  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  void setWriteCompleteCallBack(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }

  void setCloseCallBack(const CloseCallBack &cb) { closeCallBack_ = cb; }

private:
  void newConnection(Socket &&, const Inetaddress &);
  void removeConnection(const TcpConnectionPtr &);
  void removeConnectionInLoop(const TcpConnectionPtr &);

  reactor::EventLoop *loop_;
  const std::string name_;
  Connector connector_;
  int nextConnId_;
  bool retry_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  CloseCallBack closeCallBack_;
  TcpConnectionPtr tcpConnection_;
  Inetaddress serverAddr_;
  std::mutex mutex_;
  bool started;
};
} // namespace znet

#endif // !REACTOR_SRC_TCPCLIENT_H_
