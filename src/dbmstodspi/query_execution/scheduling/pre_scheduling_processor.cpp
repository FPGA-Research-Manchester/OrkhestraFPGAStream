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

using orkhestrafs::dbmstodspi::PreSchedulingProcessor;

auto PreSchedulingProcessor::GetMinimumCapacityValuesFromHWLibrary(
    const std::map<QueryOperationType, OperationPRModules>& hw_library)
    -> std::map<QueryOperationType, std::vector<int>> {
  std::map<QueryOperationType, std::vector<int>> min_capacity_map;
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

auto PreSchedulingProcessor::GetNewAvailableNodesAfterSchedulingGivenNode(
    std::string node_name, const std::vector<std::string>& past_nodes,
    const std::map<std::string, SchedulingQueryNode>& graph)
    -> std::vector<std::string> {
  std::vector<std::string> potential_nodes = graph.at(node_name).after_nodes;
  for (const auto& potential_node_name : graph.at(node_name).after_nodes) {
    for (const auto& [previous_node_name, node_index] :
         graph.at(potential_node_name).before_nodes) {
      if (std::find(past_nodes.begin(), past_nodes.end(), previous_node_name) ==
          past_nodes.end()) {
        auto search = std::find(potential_nodes.begin(), potential_nodes.end(),
                                previous_node_name);
        if (search != potential_nodes.end()) {
          potential_nodes.erase(search);
        }
      }
    }
  }
  return potential_nodes;
}

auto PreSchedulingProcessor::GetMinRequirementsForFullyExecutingNode(
    std::string node_name,
    const std::map<std::string, SchedulingQueryNode>& graph,
    AcceleratorLibraryInterface& accelerator_library,
    const std::map<std::string, TableMetadata> data_tables)
    -> std::vector<int> {
  if (accelerator_library.IsOperationSorting(graph.at(node_name).operation)) {
    auto tables_to_be_sorted = graph.at(node_name).data_tables;
    if (tables_to_be_sorted.size() != 1) {
      throw std::runtime_error(
          "Currently sorters only support one input table!");
    }
    return accelerator_library.GetMinSortingRequirements(
        graph.at(node_name).operation,
        data_tables.at(tables_to_be_sorted.front()));
  } else {
    return graph.at(node_name).capacity;
  }
}

auto PreSchedulingProcessor::FindAdequateBitstreams(
    const std::vector<int> min_requirements, QueryOperationType operation,
    const std::map<QueryOperationType, OperationPRModules>& hw_library)
    -> std::vector<std::string> {
  std::vector<std::string> fitting_bitstreams;
  for (const auto& [bitstream_name, bitstream_parameters] :
       hw_library.at(operation).bitstream_map) {
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
      fitting_bitstreams.push_back(bitstream_name);
    }
  }
  return fitting_bitstreams;
}

auto PreSchedulingProcessor::GetFittingBitstreamLocations(
    const std::vector<std::string>& fitting_bitstreams_names,
    const std::vector<std::vector<std::string>>& start_locations)
    -> std::vector<std::vector<std::string>> {
  std::vector<std::vector<std::string>> fitting_bitstream_locations;
  for (int location_index = 0; location_index < start_locations.size();
       location_index++) {
    fitting_bitstream_locations.push_back(start_locations.at(location_index));
    for (int bitstream_index = start_locations.at(location_index).size();
         bitstream_index >= 0; bitstream_index--) {
      if (std::find(fitting_bitstreams_names.begin(),
                    fitting_bitstreams_names.end(),
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

void PreSchedulingProcessor::AddNewTableToNextNodes(
    std::map<std::string, SchedulingQueryNode>& graph, std::string node_name,
    const std::vector<std::string>& table_names) {
  for (const auto& next_node_name : graph.at(node_name).after_nodes) {
    for (const auto& [current_node_index, current_stream_index] :
         GetCurrentNodeIndexesByName(graph, next_node_name, node_name)) {
      graph.at(next_node_name).data_tables.at(current_node_index) =
          table_names.at(current_stream_index);
    }
  }
}

auto PreSchedulingProcessor::GetCurrentNodeIndexesByName(
    const std::map<std::string, SchedulingQueryNode>& graph,
    std::string next_node_name, std::string current_node_name)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> resulting_indexes;
  for (int potential_current_node_index = 0;
       potential_current_node_index <
       graph.at(next_node_name).before_nodes.size();
       potential_current_node_index++) {
    if (graph.at(next_node_name)
            .before_nodes.at(potential_current_node_index)
            .first == current_node_name) {
      auto stream_index = graph.at(next_node_name)
                              .before_nodes.at(potential_current_node_index)
                              .second;
      resulting_indexes.push_back({potential_current_node_index, stream_index});
    }
  }
  if (resulting_indexes.empty()) {
    throw std::runtime_error(
        "No next nodes found with the expected dependency");
  }
  return resulting_indexes;
}

auto PreSchedulingProcessor::GetWorstCaseProcessedTables(
    const std::vector<std::string>& input_tables,
    AcceleratorLibraryInterface& accelerator_library,
    std::map<std::string, TableMetadata>& data_tables,
    const std::vector<int>& min_capacity) -> std::vector<std::string> {
  // For this method yh you can make the largest_input_is_output but while
  // scheduling filter and join operations are a bit of a puzzle. During
  // scheduling the worst case scenario tables are forwarded but it has to be
  // made sure that those tables are actually not used and will be updated.

  // Also need to figure out how the new table names would work? Partial sort
  // and blocking sort create new tables.
  return std::vector<std::string>();
}

void PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
    const std::vector<std::string>& available_nodes,
    std::map<std::string, SchedulingQueryNode>& graph,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    std::map<std::string, TableMetadata>& data_tables,
    AcceleratorLibraryInterface& accelerator_library) {
  // Finally add everything together.
}