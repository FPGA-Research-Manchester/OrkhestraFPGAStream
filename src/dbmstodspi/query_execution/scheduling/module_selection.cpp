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

void ModuleSelection::SelectAccordingToMode(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_placements) const {
  switch (value_) {
    case kAll:
      SelectAll(available_placements);
      break;
    case kFirst:
      SelectFirst(available_placements);
      break;
    case kLast:
      SelectLast(available_placements);
      break;
    case kShortest:
      SelectShortest(available_placements);
      break;
    case kLongest:
      SelectLongest(available_placements);
      break;
    default:
      throw std::runtime_error("Wrong placement mode!");
  };
}

void ModuleSelection::SelectAll(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_placements) {
}

void ModuleSelection::SelectFirst(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_placements) {
  int min_available_position = std::numeric_limits<int>::max();
  for (const auto& placement : available_placements) {
    if (placement.second.position.first < min_available_position) {
      min_available_position = placement.second.position.first;
    }
  }

  for (auto it = available_placements.begin();
       it != available_placements.end();) {
    if (it->second.position.first != min_available_position) {
      it = available_placements.erase(it);
    } else {
      ++it;
    }
  }
}

void ModuleSelection::SelectLast(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_placements) {
  int max_available_position = 0;
  for (const auto& placement : available_placements) {
    if (placement.second.position.first > max_available_position) {
      max_available_position = placement.second.position.first;
    }
  }

  for (auto it = available_placements.begin();
       it != available_placements.end();) {
    if (it->second.position.first != max_available_position) {
      it = available_placements.erase(it);
    } else {
      ++it;
    }
  }
}

void ModuleSelection::SelectShortest(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_placements) {
  int min_module_size = std::numeric_limits<int>::max();
  for (const auto& placement : available_placements) {
    if (placement.second.position.second - placement.second.position.first + 1 <
        min_module_size) {
      min_module_size = placement.second.position.second -
                        placement.second.position.first + 1;
    }
  }

  for (auto it = available_placements.begin();
       it != available_placements.end();) {
    if (it->second.position.second - it->second.position.first +
            1 !=
        min_module_size) {
      it = available_placements.erase(it);
    } else {
      ++it;
    }
  }
}

void ModuleSelection::SelectLongest(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_placements) {
  int max_module_size = 0;
  for (const auto& placement : available_placements) {
    if (placement.second.position.second - placement.second.position.first + 1 >
        max_module_size) {
      max_module_size = placement.second.position.second -
                        placement.second.position.first + 1;
    }
  }
  for (auto it = available_placements.begin();
       it != available_placements.end();) {
    if (it->second.position.second - it->second.position.first +
            1 !=
        max_module_size) {
      it = available_placements.erase(it);
    } else {
      ++it;
    }
  }
}