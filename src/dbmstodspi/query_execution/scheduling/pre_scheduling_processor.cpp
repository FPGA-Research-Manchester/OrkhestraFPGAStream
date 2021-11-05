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
  return std::vector<std::string>();
}

auto PreSchedulingProcessor::GetMinRequirementsForFullyExecutingNode(
    std::string node_name,
    const std::map<std::string, SchedulingQueryNode>& graph,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::map<std::string, TableMetadata> data_tables)
    -> std::vector<int> {
  return std::vector<int>();
}

auto PreSchedulingProcessor::FindAdequateBitstreams(
    const std::vector<int> min_requirements, QueryOperationType operation,
    const std::map<QueryOperationType, OperationPRModules>& hw_library)
    -> std::vector<std::string> {
  return std::vector<std::string>();
}

auto PreSchedulingProcessor::GetFittingBitstreamLocations(
    const std::vector<std::string>& fitting_bitstreams,
    const std::vector<std::vector<std::string>>& start_locations)
    -> std::vector<std::vector<std::string>> {
  return std::vector<std::vector<std::string>>();
}

void PreSchedulingProcessor::AddNewTableToNextNodes(
    std::map<std::string, SchedulingQueryNode>& graph, std::string node_name,
    const std::vector<std::string>& table_names) {}

auto PreSchedulingProcessor::GetWorstCaseProcessedTables(
    const std::vector<std::string>& input_tables,
    AcceleratorLibraryInterface& accelerator_library,
    std::map<std::string, TableMetadata>& data_tables,
    const std::vector<int>& min_capacity) -> std::vector<std::string> {
  return std::vector<std::string>();
}

void PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
    const std::vector<std::string>& available_nodes,
    std::map<std::string, SchedulingQueryNode>& graph,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    std::map<std::string, TableMetadata>& data_tables,
    AcceleratorLibraryInterface& accelerator_library) {}