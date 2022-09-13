#include "Timestamp.h"
#include <bits/types/struct_tm.h>
#include <ctime>
#include <string>

#include "AsyncLog.h"

namespace znet {
namespace timer {
Timestamp addTime(Timestamp timestamp, double second) {
  Timestamp delay = static_cast<Timestamp>(second * kMicroSecondsPerSecond);
  return timestamp + delay;
}

std::string formatTimestamp(Timestamp timestamp) {
  struct tm time;
  time_t timestampOfS = timestamp / (1000 * 1000);
  LOGDEBUG << timestamp;
  localtime_r(&timestampOfS, &time);
  char temp[32];
  std::strftime(temp, sizeof temp, "%Y-%m-%d_%H:%M:%S", &time);
  std::string date(temp);
  long micr = timestamp % (1000 * 1000);
  return date + "." + std::to_string(micr);
}

} // namespace timer
} // namespace znet
