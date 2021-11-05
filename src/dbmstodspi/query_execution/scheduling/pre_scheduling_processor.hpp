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
#include <vector>

#include "operation_types.hpp"
#include "pr_module_data.hpp"
#include "scheduling_query_node.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::hw_library::OperationPRModules;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::dbmstodspi::SchedulingQueryNode;
using orkhestrafs::core_interfaces::table_data::TableMetadata;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to preprocess the graph to improve performance
 */
class PreSchedulingProcessor {
 public:
  /**
   * @brief Method to get the smallest capacity values from the HW library
   * @param hw_library Map of PR modules available for each operation
   * @return Capacity values for each operation
   */
  static auto GetMinimumCapacityValuesFromHWLibrary(
      const std::map<QueryOperationType, OperationPRModules>& hw_library)
      -> std::map<QueryOperationType, std::vector<int>>;

  //def get_new_available_nodes(scheduled_node, past_nodes, all_nodes):
  static auto GetNewAvailableNodesAfterSchedulingGivenNode(
      std::string node_name, const std::vector<std::string>& past_nodes,
      const std::map<std::string, SchedulingQueryNode>& graph)
      -> std::vector<std::string>;

  // def get_min_requirements(current_node_name, graph, hw_library, data_tables)
  static auto GetMinRequirementsForFullyExecutingNode(
      std::string node_name,
      const std::map<std::string, SchedulingQueryNode>& graph,
      const std::map<QueryOperationType, OperationPRModules>& hw_library,
      const std::vector<TableMetadata> data_tables) -> std::vector<int>;

  // def find_adequate_bitstreams(min_requirements, operation, hw_library)
  static auto FindAdequateBitstreams(
      const std::vector<int> min_requirements, QueryOperationType operation,
      const std::map<QueryOperationType, OperationPRModules>& hw_library)
      -> std::vector<std::string>;

  // def get_fitting_bitstream_locations_based_on_list(list_of_fitting_bitstreams, start_locations)
  static auto GetFittingBitstreamLocations(
      const std::vector<std::string>& fitting_bitstreams,
      const std::vector<std::vector<std::string>>& start_locations)
      -> std::vector<std::vector<std::string>>;

  // def add_new_table_to_next_nodes_in_place(all_nodes, node, table_names)

  // def get_worst_case_fully_processed_tables(input_tables, current_node_decorators, data_tables, min_capacity)

  // def add_satisfying_bitstream_locations_to_graph(available_nodes, graph, hw_library, data_tables)
};

}  // namespace orkhestrafs::dbmstodspi