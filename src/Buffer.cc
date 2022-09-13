#include "Buffer.h"

#include <assert.h>
#include <bits/types/struct_iovec.h>
#include <cerrno>
#include <sys/uio.h>
#include <unistd.h>

namespace znet {
namespace buffer {
Buffer::Buffer()
    : data_(kInitialSize + kCheapPrepend), readIndex(kCheapPrepend),
      writeIndex(kCheapPrepend) {
  assert(0 == readAbleBytes());
  assert(kInitialSize == writeAbleBytes());
  assert(kCheapPrepend == preAbleBytes());
}

void Buffer::swap(Buffer &rhs) {
  data_.swap(rhs.data_);
  std::swap(readIndex, rhs.readIndex);
  std::swap(writeIndex, rhs.writeIndex);
}

void Buffer::retrieve(size_t len) {
  assert(len <= readAbleBytes());
  readIndex += len;
}

void Buffer::append(const char *data, size_t size) {
  ensureWriteAble(size);
  std::copy(data, data + size, data_.begin() + writeIndex);
  writeIndex += size;
}

void Buffer::append(const std::string &str) {
  append(str.c_str(), str.length());
}

void Buffer::ensureWriteAble(size_t writeSize) {
  if (writeSize <= writeAbleBytes()) {
    return;
  } else {
    makeSpace(writeSize);
  }
}

void Buffer::prepend(const void *data, size_t size) {
  assert(size < preAbleBytes());
  readIndex -= size;
  const char *ptr = static_cast<const char *>(data);
  std::copy(ptr, ptr + size, data_.begin() + readIndex);
}

void Buffer::makeSpace(size_t len) {
  if (writeAbleBytes() + preAbleBytes() < len + kCheapPrepend) {
    data_.resize(writeIndex + len);
  } else {
    assert(kCheapPrepend < readIndex);
    size_t readable = readAbleBytes();
    std::copy(data_.begin() + readIndex, data_.begin() + writeIndex,
              data_.begin() + kCheapPrepend);
    writeIndex = readable + kCheapPrepend;
    readIndex = kCheapPrepend;
    assert(readable == readAbleBytes());
  }
}

ssize_t Buffer::readFd(int fd, int *saveErr) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writeable = writeAbleBytes();
  vec[0].iov_base = &*data_.begin() + writeIndex;
  vec[0].iov_len = writeable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    *saveErr = errno;
  } else if (n <= static_cast<ssize_t>(writeable)) {
    writeIndex += n;
  } else {
    writeIndex = data_.size();
    append(extrabuf, n - writeable);
  }
  return n;
}

} // namespace buffer
} // namespace znet
