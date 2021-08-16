/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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

}  // namespace dbmstodspi::logger