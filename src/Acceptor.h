/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-08                                                   #
  # @ File: src/Socket.h                                                 #
  #                                                                      #
  # @ Description: a channel of acceptor                                 #
  ########################################################################*/

#ifndef REACTOR_SRC_ACCEPTOR_H_
#define REACTOR_SRC_ACCEPTOR_H_

#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

namespace znet {
class Acceptor {
public:
  Acceptor(reactor::EventLoop *loop, const Inetaddress &addr);

  void setNewConnectCallback(const NewConnectCallback &cb) {
    newConnectCallbcak_ = cb;
  }

  bool listenning() { return listenning_; }
  void listen();

private:
  void handleReader();

  reactor::EventLoop *loop_;
  Socket socket_;
  reactor::Channel accpectChannel_;
  bool listenning_;
  NewConnectCallback newConnectCallbcak_;
};
} // namespace znet

#endif // !REACTOR_SRC_ACCEPTOR_H_
