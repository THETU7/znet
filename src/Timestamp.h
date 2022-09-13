/*########################################################################
  # @ License: Look the Root Of Project.                                 #
  #                                                                      #
  # @ Author: Zongyang                                                   #
  # @ Data: 2022-09-06                                                   #
  # @ File: src/Timestamp.h                                              #
  #                                                                      #
  # @ Description: some funs to handle timespace                         #
  ########################################################################*/

#include "Callbacks.h"
namespace znet {
namespace timer {
// Timestamp 以微秒为单位
#define kMicroSecondsPerSecond 1000 * 1000
using Timestamp = long;

Timestamp addTime(Timestamp timestamp, double second);

std::string formatTimestamp(Timestamp timestamp);
} // namespace timer
} // namespace znet
