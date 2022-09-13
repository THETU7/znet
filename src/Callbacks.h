/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: Callbacks.h                                                  #
  #                                                                      #
  # @ Description: a define of some callbacks                            #
  ########################################################################*/

#ifndef REACTOR_SRC_CALLBACKS_H_
#define REACTOR_SRC_CALLBACKS_H_

#include <functional>
#include <memory>

#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"

namespace znet {
namespace timer {
using TimerCallBack = std::function<void()>;
using Timestamp = long;
} // namespace timer

namespace reactor {
using ReadEventCallBack = std::function<void(timer::Timestamp)>;
}

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using NewConnectCallback = std::function<void(Socket &&, const Inetaddress &)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(
    const TcpConnectionPtr &, buffer::Buffer *buffer, timer::Timestamp time)>;
using CloseCallBack = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
} // namespace znet

#endif // !REACTOR_SRC_CALLBACKS_H_
