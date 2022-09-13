/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-08                                                   #
  # @ File: src/InetAddress.h                                            #
  #                                                                      #
  # @ Description: handle the order of net address                       #
  ########################################################################*/

#ifndef REACTOR_SRC_INETADDRESS_H_
#define REACTOR_SRC_INETADDRESS_H_

#include <netinet/in.h>

#include <cstdint>
#include <string>

namespace znet {
class Inetaddress {
public:
  explicit Inetaddress(uint16_t port);
  Inetaddress(const std::string &ip, uint16_t port);
  Inetaddress(const struct sockaddr_in &addr) : addr_(addr){};
  Inetaddress(const Inetaddress &) = default;

  std::string toHostPort() const;

  const struct sockaddr_in &getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

private:
  struct sockaddr_in addr_;
};
} // namespace znet

#endif // !REACTOR_SRC_INETADDRESS_H_
