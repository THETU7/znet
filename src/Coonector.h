/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-09                                                   #
  # @ File: src/Coonector.h                                              #
  #                                                                      #
  # @ Description: a class for handle socket coonect                     #
  ########################################################################*/

#ifndef REACTOR_SRC_COONECTOR_H_
#define REACTOR_SRC_COONECTOR_H_

#include <functional>
#include <memory>

#include <boost/core/noncopyable.hpp>

#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TimerId.h"

namespace znet {
class Connector : public boost::noncopyable {
public:
  Connector(reactor::EventLoop *loop, const Inetaddress &serveraddr);
  ~Connector();

  void setNewConnectionCallback(const NewConnectCallback &cb) {
    newConnectionCallback_ = cb;
  }

  void start();
  void restart();
  void stop();

private:
  enum class States { kDisconnected, kConnecting, kConnected };
  static const int kMaxRetryDelayMs = 30 * 1000;
  static const int kInitRetryDelayMs = 500;

  void setState(States e) { state_ = e; }
  void startInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  reactor::EventLoop *loop_;
  Inetaddress serveraddr_;
  bool connect_;
  States state_;
  std::unique_ptr<reactor::Channel> channel_;
  int retryDelayMs_;
  timer::TimerId timerId_;
  NewConnectCallback newConnectionCallback_;
};
} // namespace znet

#endif // !REACTOR_SRC_COONECTOR_H_
