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

#include <map>
#include <string>
#include <utility>

#include "operation_types.hpp"
#include "scheduled_module.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::dbmstodspi::ScheduledModule;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Struct to hold data about a specific module placement during
 * scheduling.
 */
class ModuleSelection {
 private:
  enum SelectionMode { kAll, kFirst, kLast, kShortest, kLongest };
  const std::map<std::string, SelectionMode> kToStringMap = {
      {"ALL_AVAILABLE", SelectionMode::kAll},
      {"FIRST_AVAILABLE", SelectionMode::kFirst},
      {"LAST_AVAILABLE", SelectionMode::kLast},
      {"SHORTEST_AVAILABLE", SelectionMode::kShortest},
      {"LONGEST_AVAILABLE", SelectionMode::kLongest}};

  SelectionMode value_;

  // TODO: Find a more beautiful way to do this as methods are not allowed with
  // enums in C.
  static auto SelectAll(
      const std::vector<std::pair<int, ScheduledModule>>& available_placements)
      -> std::vector<std::pair<int, ScheduledModule>>;
  static auto SelectFirst(
      const std::vector<std::pair<int, ScheduledModule>>& available_placements)
      -> std::vector<std::pair<int, ScheduledModule>>;
  static auto SelectLast(
      const std::vector<std::pair<int, ScheduledModule>>& available_placements)
      -> std::vector<std::pair<int, ScheduledModule>>;
  static auto SelectShortest(
      const std::vector<std::pair<int, ScheduledModule>>& available_placements)
      -> std::vector<std::pair<int, ScheduledModule>>;
  static auto SelectLongest(
      const std::vector<std::pair<int, ScheduledModule>>& available_placements)
      -> std::vector<std::pair<int, ScheduledModule>>;

 public:
  ModuleSelection(std::string selection_mode)
      : value_{kToStringMap.at(selection_mode)} {};
  auto SelectAccordingToMode(
      const std::vector<std::pair<int, ScheduledModule>>& available_placements) const
      -> std::vector<std::pair<int, ScheduledModule>>;
};

}  // namespace orkhestrafs::dbmstodspi