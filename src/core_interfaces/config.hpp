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

#include "pr_module_data.hpp"
#include "query_scheduling_data.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::hw_library::OperationPRModules;
using orkhestrafs::core_interfaces::operation_types::QueryOperation;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::core_interfaces::table_data::TableMetadata;

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

  /*int frame_size = 372;
  std::map<char, int> cost_of_columns = {{'M', 216 * frame_size},
                                         {'D', 200 * frame_size},
                                         {'B', 196 * frame_size}};*/
  std::map<char, int> cost_of_columns;

  // Default values here:
  bool reduce_single_runs = true;
  bool use_max_runs_cap = true;
  bool prioritise_children = true;
  int heuristic_choice = 0;

  double streaming_speed = 4800000000;
  double configuration_speed = 66000000;

  double time_limit_duration_in_seconds = -1;

  std::string resource_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM";

  double utilites_scaler = -1;
  double config_written_scaler = -1;
  double utility_per_frame_scaler = -1;

  /// CSV data column separator character.
  char csv_separator;

  std::vector<std::string> debug_forced_pr_bitstreams;
};
}  // namespace orkhestrafs::core_interfaces