/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-08                                                   #
  # @ File: src/InetAddress.h                                            #
  #                                                                      #
  # @ Description: some funs of Socket                                   #
  ########################################################################*/

#ifndef REACTOR_SRC_SOCKETSOPT_H_
#define REACTOR_SRC_SOCKETSOPT_H_

#include <endian.h>
#include <netinet/in.h>

#include <cstdint>

namespace znet {
namespace sockets {
typedef struct sockaddr SA;

inline uint64_t hostToNetwork64(uint64_t host64) { return htobe64(host64); }

inline uint32_t hostToNetwork32(uint32_t host32) { return htobe32(host32); }

inline uint16_t hostToNetwork16(uint16_t host16) { return ntohs(host16); }

inline uint64_t networkToHost64(uint64_t net64) { return be64toh(net64); }

inline uint32_t networkToHost32(uint32_t net32) { return ntohl(net32); }

inline uint16_t networkToHost16(uint16_t net16) { return ntohs(net16); }

void fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

void toHostPort(char *buf, size_t size, const struct sockaddr_in *addr);

void bindOrDie(int sockfd, const struct sockaddr_in &addr);

void listeningOrDie(int socket);
int accept(int sockfd, struct sockaddr_in *addr);

SA *sockaddr_cast(struct sockaddr_in *addr);
int createNonblockingOrDie();

int getSocketError(int sockfd) noexcept;

struct sockaddr_in getLocalAddr(int sockfd);

void shutDownWrite(int sockfd);
int connect(int fd, const struct sockaddr_in &addr);
void close(int fd);
bool isSelfConnect(int sockfd);

} // namespace sockets
} // namespace znet

#endif // !REACTOR_SRC_SOCKETSOPT_H_
