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

#include "module_selection.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

using orkhestrafs::dbmstodspi::ModuleSelection;

auto ModuleSelection::SelectAccordingToMode(
    const std::vector<std::pair<int, ScheduledModule>>& available_placements)
    const -> std::vector<std::pair<int, ScheduledModule>> {
  switch (value_) {
    case kAll:
      return SelectAll(available_placements);
      break;
    case kFirst:
      return SelectFirst(available_placements);
      break;
    case kLast:
      return SelectLast(available_placements);
      break;
    case kShortest:
      return SelectShortest(available_placements);
      break;
    case kLongest:
      return SelectLongest(available_placements);
      break;
    default:
      throw std::runtime_error("Wrong placement mode!");
  };
}

auto ModuleSelection::SelectAll(
    const std::vector<std::pair<int, ScheduledModule>>& available_placements)
    -> std::vector<std::pair<int, ScheduledModule>> {
  return available_placements;
}

auto ModuleSelection::SelectFirst(
    const std::vector<std::pair<int, ScheduledModule>>& available_placements)
    -> std::vector<std::pair<int, ScheduledModule>> {
  int min_available_position = std::numeric_limits<int>::max();
  for (const auto& placement : available_placements) {
    if (placement.second.position.first < min_available_position) {
      min_available_position = placement.second.position.first;
    }
  }
  std::vector<std::pair<int, ScheduledModule>> chosen_placements;
  for (const auto& placement : available_placements) {
    if (placement.second.position.first == min_available_position) {
      chosen_placements.push_back(placement);
    }
  }
  auto it = std::unique(chosen_placements.begin(), chosen_placements.end());
  chosen_placements.resize(std::distance(chosen_placements.begin(), it));
  return chosen_placements;
}

auto ModuleSelection::SelectLast(
    const std::vector<std::pair<int, ScheduledModule>>& available_placements)
    -> std::vector<std::pair<int, ScheduledModule>> {
  int max_available_position = 0;
  for (const auto& placement : available_placements) {
    if (placement.second.position.first > max_available_position) {
      max_available_position = placement.second.position.first;
    }
  }
  std::vector<std::pair<int, ScheduledModule>> chosen_placements;
  for (const auto& placement : available_placements) {
    if (placement.second.position.first == max_available_position) {
      chosen_placements.push_back(placement);
    }
  }
  auto it = std::unique(chosen_placements.begin(), chosen_placements.end());
  chosen_placements.resize(std::distance(chosen_placements.begin(), it));
  return chosen_placements;
}

auto ModuleSelection::SelectShortest(
    const std::vector<std::pair<int, ScheduledModule>>& available_placements)
    -> std::vector<std::pair<int, ScheduledModule>> {
  int min_module_size = std::numeric_limits<int>::max();
  for (const auto& placement : available_placements) {
    if (placement.second.position.second - placement.second.position.first + 1 <
        min_module_size) {
      min_module_size = placement.second.position.second -
                        placement.second.position.first + 1;
    }
  }
  std::vector<std::pair<int, ScheduledModule>> chosen_placements;
  for (const auto& placement : available_placements) {
    if (placement.second.position.second - placement.second.position.first +
            1 ==
        min_module_size) {
      chosen_placements.push_back(placement);
    }
  }
  auto it = std::unique(chosen_placements.begin(), chosen_placements.end());
  chosen_placements.resize(std::distance(chosen_placements.begin(), it));
  return chosen_placements;
}

auto ModuleSelection::SelectLongest(
    const std::vector<std::pair<int, ScheduledModule>>& available_placements)
    -> std::vector<std::pair<int, ScheduledModule>> {
  int max_module_size = 0;
  for (const auto& placement : available_placements) {
    if (placement.second.position.second - placement.second.position.first + 1 >
        max_module_size) {
      max_module_size = placement.second.position.first;
    }
  }
  std::vector<std::pair<int, ScheduledModule>> chosen_placements;
  for (const auto& placement : available_placements) {
    if (placement.second.position.second - placement.second.position.first +
            1 ==
        max_module_size) {
      chosen_placements.push_back(placement);
    }
  }
  auto it = std::unique(chosen_placements.begin(), chosen_placements.end());
  chosen_placements.resize(std::distance(chosen_placements.begin(), it));
  return chosen_placements;
}