/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include <memory>

#if !defined(SPDLOG_ACTIVE_LEVEL)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include "spdlog/spdlog.h"

namespace vi {

class RTCLogSink;

class Logger {
public:
  Logger();

  ~Logger();

  static void init(const std::string &log_path);

  static void destroy();

  static std::shared_ptr<spdlog::logger> &rtcLogger();

  static std::shared_ptr<spdlog::logger> &appLogger();

private:
  static std::shared_ptr<spdlog::logger> _appLogger;

  static std::shared_ptr<spdlog::logger> _rtcLogger;

  static std::unique_ptr<RTCLogSink> _rtcLogSink;
};

} // namespace vi

#define TLOG_RTC(...) SPDLOG_LOGGER_TRACE(vi::Logger::rtcLogger(), __VA_ARGS__)
#define DLOG_RTC(...) SPDLOG_LOGGER_DEBUG(vi::Logger::rtcLogger(), __VA_ARGS__)
#define ILOG_RTC(...) SPDLOG_LOGGER_INFO(vi::Logger::rtcLogger(), __VA_ARGS__)
#define WLOG_RTC(...) SPDLOG_LOGGER_WARN(vi::Logger::rtcLogger(), __VA_ARGS__)
#define ELOG_RTC(...) SPDLOG_LOGGER_ERROR(vi::Logger::rtcLogger(), __VA_ARGS__)
#define CLOG_RTC(...)                                                          \
  SPDLOG_LOGGER_CRITICAL(vi::Logger::rtcLogger(), __VA_ARGS__)

#define TLOG(...) SPDLOG_LOGGER_TRACE(vi::Logger::appLogger(), __VA_ARGS__)
#define DLOG(...) SPDLOG_LOGGER_DEBUG(vi::Logger::appLogger(), __VA_ARGS__)
#define ILOG(...) SPDLOG_LOGGER_INFO(vi::Logger::appLogger(), __VA_ARGS__)
#define WLOG(...) SPDLOG_LOGGER_WARN(vi::Logger::appLogger(), __VA_ARGS__)
#define ELOG(...) SPDLOG_LOGGER_ERROR(vi::Logger::appLogger(), __VA_ARGS__)
#define CLOG(...) SPDLOG_LOGGER_CRITICAL(vi::Logger::appLogger(), __VA_ARGS__)
