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
        fitting_bitstreams,
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

auto PreSchedulingProcessor::SetWorstCaseProcessedTables(
    const std::vector<std::string>& input_table_names,
    const std::vector<int>& min_capacity,
    std::map<std::string, TableMetadata>& data_tables,
    QueryOperationType operation,
    const std::vector<std::string>& output_table_names) -> bool {
  // Assume that there are no duplicates in input_tables.
  std::map<std::string, TableMetadata> new_tables =
      accelerator_library_.GetWorstCaseProcessedTables(
          operation, min_capacity, input_table_names, data_tables,
          output_table_names);

  if (new_tables.size() != output_table_names.size()) {
    throw std::runtime_error("Unexpected worst case tables generated!");
  }

  bool tables_updated = false;
  for (const auto& table_name : output_table_names) {
    if (new_tables.find(table_name) == new_tables.end()) {
      throw std::runtime_error("Unexpected worst case table generated!");
    }
    if (data_tables.at(table_name) != new_tables.at(table_name)) {
      tables_updated = true;
      data_tables[table_name] = new_tables.at(table_name);
    }
  }
  return tables_updated;
}

void PreSchedulingProcessor::UpdateOnlySatisfyingBitstreams(
    const std::string& node_name,
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::map<std::string, TableMetadata>& data_tables) {
  FindAdequateBitstreams(
      GetMinRequirementsForFullyExecutingNode(node_name, graph, data_tables),
      graph, node_name);
}

auto PreSchedulingProcessor::SetWorstCaseNodeCapacity(
    const std::string& node_name,
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::vector<int>& min_capacity) -> bool {
  // What I want is given the min_capacity and current tables, operation and
  // next nodes operation is capacity values given back.
  const auto& current_node = graph.at(node_name);
  if (current_node.after_nodes.size() != 1) {
    throw std::runtime_error("Multiple output nodes aren't supported!");
  }
  if (!current_node.after_nodes.front().empty()){
    auto& next_node = graph.at(current_node.after_nodes.front());
    auto capacity_values = accelerator_library_.GetWorstCaseNodeCapacity(
        current_node.operation, min_capacity, current_node.data_tables,
        data_tables, next_node.operation);
    // This check is not needed as some capacity values can be 0.
    /*if (capacity_values.size() != next_node.capacity.size()) {
        throw std::runtime_error("Incorrect number of capacity values
    calculated!");
    }*/
    bool updated_capacity_values = false;
    if (!capacity_values.empty()) {
      for (int capacity_id = 0; capacity_id < capacity_values.size();
           capacity_id++) {
        if (capacity_id >= next_node.capacity.size()) {
          next_node.capacity.push_back(capacity_values.at(capacity_id));
          updated_capacity_values = true;
        } else if (capacity_values.at(capacity_id) !=
                   next_node.capacity.at(capacity_id)) {
          next_node.capacity[capacity_id] = capacity_values.at(capacity_id);
          updated_capacity_values = true;
        }
      }
    }
    return updated_capacity_values;
  }
  return false;
}

// TODO(Kaspar): Need a special case check of 0 rows left - Can immediately cut
// stuff out.
void PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& data_tables,
    std::unordered_set<std::string>& available_nodes,
    std::unordered_set<std::string>& processed_nodes) {
  // Should know which table was updated, which node was updated.
  auto current_available_nodes = available_nodes;
  auto originally_processed_nodes = processed_nodes;
  auto current_processed_nodes = processed_nodes;
  // While there are available nodes
  while (!current_available_nodes.empty()) {
    // Get a random available node
    auto current_node_name = *current_available_nodes.begin();
    // Remove chosen node from available nodes
    current_available_nodes.erase(current_available_nodes.begin());
    // Mark it processed
    current_processed_nodes.insert(current_node_name);
    // Find new available nodes after processing current node
    QuerySchedulingHelper::UpdateAvailableNodesAfterSchedulingGivenNode(
        current_node_name, current_processed_nodes, graph,
        current_available_nodes);
    // Find what are the minimum requirements for executing current node.
    auto min_requirements = GetMinRequirementsForFullyExecutingNode(
        current_node_name, graph, data_tables);
    // Find all bitstreams that meet minimum requirements and update graph.
    FindAdequateBitstreams(min_requirements, graph, current_node_name);

    auto worst_case_table_updated = SetWorstCaseProcessedTables(
        graph.at(current_node_name).data_tables,
        min_capacity_.at(graph.at(current_node_name).operation), data_tables,
        graph.at(current_node_name).operation,
        graph.at(current_node_name)
            .node_ptr->given_output_data_definition_files);
    auto worst_case_capacity_updated = SetWorstCaseNodeCapacity(
        current_node_name, graph, data_tables,
        min_capacity_.at(graph.at(current_node_name).operation));
    // Lazily updating merge sort capacity if there is no linear sort before hand!
    // This will reduce the requirements given a different linear sort choice or filtering.
    if (graph.at(current_node_name).capacity != min_requirements) {
      graph.at(current_node_name).capacity = min_requirements;
      worst_case_capacity_updated = true; 
    }

    // A module can't be skipped if no new table was added
    if (!worst_case_table_updated && !worst_case_capacity_updated) {
      QuerySchedulingHelper::SetAllNodesAsProcessedAfterGivenNode(
          current_node_name, current_processed_nodes, graph,
          current_available_nodes);
      // TODO(Kaspar): Remove this check!
      if (min_requirements.size() == 1 && min_requirements.front() == 0) {
        throw std::runtime_error("Something went wrong!");
      }
    }
    // If a module can be skipped the min_requirements is marked as 0
    else if (min_requirements.size() == 1 && min_requirements.front() == 0) {
      if (graph.at(current_node_name).after_nodes.size() != 1 ||
          graph.at(current_node_name).before_nodes.size() != 1) {
        throw std::runtime_error(
            "Can't skip node with multiple inputs or outputs!");
      }
      // Move input table name to outputs input table.
      const auto& after_node = graph.at(current_node_name).after_nodes.front();
      if (!after_node.empty()) {
        int index = QuerySchedulingHelper::GetCurrentNodeIndexesByName(
                        graph, after_node, current_node_name)
                        .front()
                        .first;
        graph.at(after_node)
            .node_ptr->given_input_data_definition_files[index] =
            graph.at(current_node_name)
                .node_ptr->given_input_data_definition_files.front();
      }

      current_processed_nodes.insert(current_node_name);
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