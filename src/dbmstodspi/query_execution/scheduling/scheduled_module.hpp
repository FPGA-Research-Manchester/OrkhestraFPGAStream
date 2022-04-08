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
#include <utility>

#include "operation_types.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::SortedSequence;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Struct to hold data about a specific module placement during
 * scheduling.
 */
struct ScheduledModule {
  std::string node_name;
  QueryOperationType operation_type;
  std::string bitstream;
  std::pair<int, int> position;
  std::vector<SortedSequence> processed_table_data;

  // For map to work.
  auto operator<(const ScheduledModule& rhs) const -> bool {
    return position.first < rhs.position.first;
  }

  // For std::unique to work.
  auto operator==(const ScheduledModule& rhs) const -> bool {
    return node_name == rhs.node_name && operation_type == rhs.operation_type &&
           bitstream == rhs.bitstream && position == rhs.position;
  }
};

}  // namespace orkhestrafs::dbmstodspi