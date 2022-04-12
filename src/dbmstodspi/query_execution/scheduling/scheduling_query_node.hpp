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
#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Struct to hold query graph data
 */
struct SchedulingQueryNode {
  QueryOperationType operation;
  std::vector<int> capacity;
  std::vector<std::pair<std::string, int>> before_nodes;
  std::vector<std::string> after_nodes;
  std::vector<std::string> data_tables;
  std::vector<std::vector<std::string>> satisfying_bitstreams;
};
}  // namespace orkhestrafs::dbmstodspi