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
#include <vector>

#include "query_scheduling_data.hpp"
#include "table_data.hpp"
#include "pr_module_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperation;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::core_interfaces::table_data::TableMetadata;
using orkhestrafs::core_interfaces::hw_library::OperationPRModules;

namespace orkhestrafs::core_interfaces {
struct Config {
  /// Map of hardware modules and the available variations with different
  /// computational capacity values.
  std::map<QueryOperationType, std::vector<std::vector<int>>> module_library;

  /// Map of bitstreams where a combination of modules corresponds to a
  /// bitstream.
  std::map<std::vector<QueryOperation>, std::string> accelerator_library;

  /// Map telling FOS how much memory mapped register space is available for
  /// each bitstream.
  std::map<std::string, int> required_memory_space;
  /// Map telling how big each instance of a specifc data type is.
  std::map<ColumnDataType, double> data_sizes;

  // Data for the PR capable scheduler.
  std::map<std::string, TableMetadata> initial_all_tables_metadata;
  std::map<QueryOperationType, OperationPRModules> pr_hw_library;

  /// CSV data column separator character.
  char csv_separator;
};
}  // namespace orkhestrafs::core_interfaces