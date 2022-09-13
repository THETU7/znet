#include "EventLoopThreadPool.h"

#include <assert.h>

namespace znet {
EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop)
    : baseloop_(baseloop), started_(false), numThreads_(0), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start() {
  assert(baseloop_);
  baseloop_->assertInLoopThread();

  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    EvenetThreadPtr t = std::make_unique<EvenetThread>();
    loops_.push_back(t->startLoop());
    threads_.emplace_back(std::move(t));
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  baseloop_->assertInLoopThread();
  EventLoop *loop = baseloop_;

  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

} // namespace znet
