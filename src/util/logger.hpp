#pragma once
#include <string>

#include "spdlog/spdlog.h"

namespace dbmstodspi::logger {

enum class LogLevel {
  kTrace = spdlog::level::trace,
  kDebug = spdlog::level::debug,
  kInfo = spdlog::level::info,
  kWarning = spdlog::level::warn,
  kError = spdlog::level::err,
  kCritical = spdlog::level::critical,
  kOff = spdlog::level::off
};

inline void SetLoggingLevel(LogLevel global_level) {
  spdlog::set_level(static_cast<spdlog::level::level_enum>(global_level));
}

inline void Log(LogLevel global_level, std::string message) {
  spdlog::log(static_cast<spdlog::level::level_enum>(global_level), message);
}

inline auto ShouldLog(LogLevel log_level) {
  return spdlog::should_log(static_cast<spdlog::level::level_enum>(log_level));
}

}  // namespace dbmstodspi::util