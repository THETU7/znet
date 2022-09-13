#include "EventLoopThread.h"

#include <assert.h>

namespace znet {

EvenetThread::EvenetThread() : loop_(nullptr), existing_(false) {}

reactor::EventLoop *EvenetThread::startLoop() {
  thread_ = std::thread([ptr = this]() { ptr->threadFun(); });
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) {
      cond_.wait(lock);
    }
  }
  return loop_;
}

void EvenetThread::threadFun() {
  reactor::EventLoop loop;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }
  loop_->loop();
}

EvenetThread::~EvenetThread() {
  existing_ = true;
  loop_->quit();
  thread_.join();
}

} // namespace znet
