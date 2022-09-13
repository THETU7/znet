#include "InetAddress.h"

#include <strings.h>

#include "SocketsOps.h"

namespace znet {

static const in_addr_t kInaddrAny = INADDR_ANY;

Inetaddress::Inetaddress(uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = sockets::hostToNetwork32(kInaddrAny);
  addr_.sin_port = sockets::hostToNetwork16(port);
}

Inetaddress::Inetaddress(const std::string &ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  sockets::fromHostPort(ip.c_str(), port, &addr_);
}

std::string Inetaddress::toHostPort() const {
  char buf[32];
  sockets::toHostPort(buf, sizeof buf, &addr_);
  return buf;
}

} // namespace znet
