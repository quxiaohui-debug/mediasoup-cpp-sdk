/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#include "rtc_log_sink.h"
#include "spd_logger.h"

namespace vi {
void RTCLogSink::OnLogMessage(const std::string &msg,
                              rtc::LoggingSeverity severity, const char *tag) {}

void RTCLogSink::OnLogMessage(const std::string &message,
                              rtc::LoggingSeverity severity) {
  switch (severity) {
  case rtc::LoggingSeverity::LS_VERBOSE:
    TLOG_RTC(message);
    break;
  case rtc::LoggingSeverity::LS_INFO:
    ILOG_RTC(message);
    break;
  case rtc::LoggingSeverity::LS_WARNING:
    WLOG_RTC(message);
    break;
  case rtc::LoggingSeverity::LS_ERROR:
    ELOG_RTC(message);
    break;
  default:
    break;
  }
}

void RTCLogSink::OnLogMessage(const std::string &message) {}
} // namespace vi
