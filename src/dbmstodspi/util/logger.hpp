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

/**
 * @brief Namespace for logging methods using https://github.com/gabime/spdlog.
 */
namespace orkhestrafs::dbmstodspi::logging {

/**
 * @brief Logging level enums.
 */
enum class LogLevel {
  kTrace = spdlog::level::trace,
  kDebug = spdlog::level::debug,
  kInfo = spdlog::level::info,
  kWarning = spdlog::level::warn,
  kError = spdlog::level::err,
  kCritical = spdlog::level::critical,
  kOff = spdlog::level::off
};

/**
 * @brief Set global logging level.
 * @param global_level Which importance printted log messages have to have.
 */
inline void SetLoggingLevel(LogLevel global_level) {
  spdlog::set_level(static_cast<spdlog::level::level_enum>(global_level));
}

/**
 * @brief Log messages with a certain priority.
 * @param log_level Importance of the logging message.
 * @param message The log message recorded.
 */
inline void Log(LogLevel log_level, std::string message) {
  spdlog::log(static_cast<spdlog::level::level_enum>(log_level), message);
}

/**
 * @brief Check if the given log level is high enough to have log messages
 * appear.
 * @param log_level Importance of the logging message.
 * @return Boolean flag showing if the log level is high enough.
 */
inline auto ShouldLog(LogLevel log_level) {
  return spdlog::should_log(static_cast<spdlog::level::level_enum>(log_level));
}

}  // namespace orkhestrafs::dbmstodspi::logging