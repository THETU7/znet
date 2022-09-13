/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-09                                                   #
  # @ File: src/Buffer.h                                                 #
  #                                                                      #
  # @ Description: a Buffer for socket writer and reader                 #
  ########################################################################*/

#ifndef REACTOR_SRC_BUFFER_H_
#define REACTOR_SRC_BUFFER_H_

#include <string>
#include <vector>

namespace znet {
namespace buffer {

class Buffer {
public:
  static const int kCheapPrepend = 8;
  static const int kInitialSize = 1024;

  Buffer();
  Buffer(const Buffer &) = default;
  Buffer(Buffer &&) = default;
  size_t readAbleBytes() { return writeIndex - readIndex; }
  size_t writeAbleBytes() { return data_.size() - writeIndex; }
  size_t preAbleBytes() { return readIndex; }
  const char *peek() { return data_.data() + readIndex; }
  void retrieve(size_t);
  void retrieveAll() {
    readIndex = kCheapPrepend;
    writeIndex = kCheapPrepend;
  };
  std::string retrieveAsString() {
    std::string re(peek(), readAbleBytes());
    retrieveAll();
    return re;
  };
  void append(const std::string &str);
  void append(const char *, size_t size);
  void prepend(const void *, size_t size);
  void swap(Buffer &);
  ssize_t readFd(int fd, int *saveErr);
  void ensureWriteAble(size_t writeSize);
  void makeSpace(size_t len);

private:
  std::vector<char> data_;
  size_t readIndex;
  size_t writeIndex;
};
} // namespace buffer
} // namespace znet

#endif // !REACTOR_SRC_BUFFER_H_
