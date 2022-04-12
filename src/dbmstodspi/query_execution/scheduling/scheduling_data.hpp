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
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "scheduling_query_node.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::table_data::TableMetadata;
using orkhestrafs::dbmstodspi::SchedulingQueryNode;

namespace orkhestrafs::dbmstodspi::scheduling_data {

struct ExecutionPlanSchedulingData {
  std::unordered_set<std::string> processed_nodes;
  std::unordered_set<std::string> available_nodes;
  std::unordered_map<std::string, SchedulingQueryNode> graph;
  std::map<std::string, TableMetadata> tables;
  int streamed_data_size;
};

}  // namespace orkhestrafs::dbmstodspi::scheduling_data