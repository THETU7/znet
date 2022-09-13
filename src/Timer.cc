#include "Timer.h"

namespace znet {
namespace timer {

std::atomic_int64_t Timer::s_numCreated;

void Timer::restart(Timestamp now) {
  if (repeat_) {
    expiration_ = addTime(now, interval_);
  } else {
    expiration_ = now;
  }
}
} // namespace timer
} // namespace znet
