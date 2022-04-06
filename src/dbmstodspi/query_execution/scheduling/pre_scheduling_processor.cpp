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

#include "pre_scheduling_processor.hpp"

#include <algorithm>
#include <stdexcept>

#include "query_scheduling_helper.hpp"

using orkhestrafs::dbmstodspi::PreSchedulingProcessor;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

auto PreSchedulingProcessor::GetMinimumCapacityValuesFromHWLibrary(
    const std::map<QueryOperationType, OperationPRModules>& hw_library)
    -> std::unordered_map<QueryOperationType, std::vector<int>> {
  std::unordered_map<QueryOperationType, std::vector<int>> min_capacity_map;
  for (const auto& [operation, operation_bitstreams_data] : hw_library) {
    for (const auto& [bitstream_name, bitstream_parameters] :
         operation_bitstreams_data.bitstream_map) {
      const auto& [operation_iterator, inserted_flag] =
          min_capacity_map.try_emplace(operation,
                                       bitstream_parameters.capacity);
      if (!inserted_flag) {
        for (int capacity_param_index = 0;
             capacity_param_index < bitstream_parameters.capacity.size();
             capacity_param_index++) {
          if (operation_iterator->second.at(capacity_param_index) >
              bitstream_parameters.capacity.at(capacity_param_index)) {
            operation_iterator->second[capacity_param_index] =
                bitstream_parameters.capacity.at(capacity_param_index);
          }
        }
      }
    }
  }
  return min_capacity_map;
}

auto PreSchedulingProcessor::GetMinRequirementsForFullyExecutingNode(
    const std::string& node_name,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::map<std::string, TableMetadata>& data_tables)
    -> std::vector<int> {
  if (accelerator_library_.IsOperationSorting(graph.at(node_name).operation)) {
    auto tables_to_be_sorted = graph.at(node_name).data_tables;
    if (tables_to_be_sorted.size() != 1) {
      throw std::runtime_error(
          "Currently sorters only support one input table!");
    }
    return accelerator_library_.GetMinSortingRequirements(
        graph.at(node_name).operation,
        data_tables.at(tables_to_be_sorted.front()));
  }
  return graph.at(node_name).capacity;
}

void PreSchedulingProcessor::FindAdequateBitstreams(
    const std::vector<int>& min_requirements,
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::string& node_name) {
  std::unordered_set<std::string> fitting_bitstreams;
  for (const auto& [bitstream_name, bitstream_parameters] :
       hw_library_.at(graph.at(node_name).operation).bitstream_map) {
    bool bitstream_utility_is_great_enough = true;
    for (int capacity_parameter_index = 0;
         capacity_parameter_index < bitstream_parameters.capacity.size();
         capacity_parameter_index++) {
      if (bitstream_parameters.capacity.at(capacity_parameter_index) <
          min_requirements.at(capacity_parameter_index)) {
        bitstream_utility_is_great_enough = false;
        break;
      }
    }
    if (bitstream_utility_is_great_enough) {
      fitting_bitstreams.insert(bitstream_name);
    }
  }
  if (!fitting_bitstreams.empty()) {
    graph.at(node_name).satisfying_bitstreams = GetFittingBitstreamLocations(
        std::move(fitting_bitstreams),
        hw_library_.at(graph.at(node_name).operation).starting_locations);
  }
}

auto PreSchedulingProcessor::GetFittingBitstreamLocations(
    const std::unordered_set<std::string>& fitting_bitstreams_names,
    const std::vector<std::vector<std::string>>& start_locations)
    -> std::vector<std::vector<std::string>> {
  std::vector<std::vector<std::string>> fitting_bitstream_locations =
      start_locations;
  for (int location_index = 0; location_index < start_locations.size();
       location_index++) {
    for (int bitstream_index = start_locations.at(location_index).size() - 1;
         bitstream_index >= 0; bitstream_index--) {
      if (fitting_bitstreams_names.find(
              start_locations.at(location_index).at(bitstream_index)) ==
          fitting_bitstreams_names.end()) {
        fitting_bitstream_locations.at(location_index)
            .erase(fitting_bitstream_locations.at(location_index).begin() +
                   bitstream_index);
      }
    }
  }
  return fitting_bitstream_locations;
}

auto PreSchedulingProcessor::GetWorstCaseProcessedTables(
    const std::vector<std::string>& input_tables, const std::vector<int>& min_capacity,
    std::map<std::string, TableMetadata>& data_tables, QueryOperationType operation)
    -> std::vector<std::string> {
  std::map<std::string, TableMetadata> new_tables =
      accelerator_library_.GetWorstCaseProcessedTables(
          operation, min_capacity, input_tables, data_tables);

  std::vector<std::string> table_names;
  table_names.reserve(new_tables.size());
  for (const auto& [key, _] : new_tables) {
    table_names.push_back(key);
  }
  data_tables.merge(new_tables);
  return table_names;
}

// TODO: Need a special case check of 0 rows left - Can immediately cut stuff out.
void PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& data_tables,
    std::unordered_set<std::string>& available_nodes,
    std::unordered_set<std::string> processed_nodes) {
  // Should know which table was updated, which node was updated.
  auto current_available_nodes = available_nodes;
  auto originally_processed_nodes = processed_nodes;
  // While there are available nodes
  while (!current_available_nodes.empty()) {
    // Get a random available node
    auto current_node_name = *current_available_nodes.begin();
    // Remove chosen node from available nodes
    current_available_nodes.erase(current_available_nodes.begin());
    // Mark it processed
    processed_nodes.insert(current_node_name);
    // Find new available nodes after processing current node
    QuerySchedulingHelper::UpdateAvailableNodesAfterSchedulingGivenNode(
        current_node_name, processed_nodes, graph, current_available_nodes);
    // Find what are the minimum requirements for executing current node.
    auto min_requirements = GetMinRequirementsForFullyExecutingNode(
        current_node_name, graph, data_tables);
    // Find all bitstreams that meet minimum requirements and update graph.
    FindAdequateBitstreams(min_requirements, graph,
                           current_node_name);
    // Get worst case tables and update the graph - Because fitting ones might not have been used.
    // If new tables are the exact same as old tables - don't do anything
    // Set following stuff as processed and not available! -> Nice and universal!
    auto new_table_added = QuerySchedulingHelper::AddNewTableToNextNodes(
        graph, current_node_name,
        GetWorstCaseProcessedTables(
            graph.at(current_node_name).data_tables,
            min_capacity_.at(graph.at(current_node_name).operation), data_tables,
            graph.at(current_node_name).operation));
    // A module can't be skipped if no new table was added
    if (!new_table_added){
      QuerySchedulingHelper::SetAllNodesAsProcessedAfterGivenNode(current_node_name, processed_nodes, graph, current_available_nodes);
      // TODO: Remove this check!
      if (min_requirements.size() == 1 && min_requirements.front() == 0) {
        throw std::runtime_error("Something went wrong!");
      }
    }
    // If a module can be skipped the min_requirements is marked as 0
    else if (min_requirements.size() == 1 && min_requirements.front() == 0) {
      if (available_nodes.find(current_node_name) != available_nodes.end()) {
        available_nodes.erase(current_node_name);
        auto processed_nodes_with_deleted_nodes = originally_processed_nodes;
        processed_nodes_with_deleted_nodes.insert(current_node_name);
        QuerySchedulingHelper::UpdateAvailableNodesAfterSchedulingGivenNode(
            current_node_name, processed_nodes_with_deleted_nodes, graph,
            available_nodes);
      }
      QuerySchedulingHelper::RemoveNodeFromGraph(graph, current_node_name);
    }
  }
}