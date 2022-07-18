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

#include "acceleration_module_setup_interface.hpp"

#include <stdexcept>

using orkhestrafs::dbmstodspi::AccelerationModuleSetupInterface;

auto AccelerationModuleSetupInterface::IsMultiChannelStream(
    bool /*is_input_stream*/, int /*stream_index*/) -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::GetMultiChannelParams(
    bool /*is_input*/, int /*stream_index*/,
    std::vector<std::vector<int>> /*operation_parameters*/)
    -> std::pair<int, std::vector<int>> {
  return {-1, {-1}};
}

auto AccelerationModuleSetupInterface::GetStreamRecordSize(
    const StreamDataParameters& stream_parameters) -> int {
  if (stream_parameters.stream_specification.empty()) {
    return stream_parameters.stream_record_size;
  }
  return stream_parameters.stream_specification.size();
}

auto AccelerationModuleSetupInterface::IsConstrainedToFirstInPipeline()
    -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::GetCapacityRequirement(
    std::vector<std::vector<int>> /*operation_parameters*/)
    -> std::vector<int> {
  return {};
}

auto AccelerationModuleSetupInterface::IsSortingInputTable() -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::GetWorstCaseProcessedTables(
    const std::vector<int>& /*min_capacity*/,
    const std::vector<std::string>& input_tables,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::vector<std::string>& output_table_names)
    -> std::map<std::string, TableMetadata> {
  if (input_tables.size() != 1 || output_table_names.size() != 1) {
    throw std::runtime_error("Unsupporded table counts at preprocessing!");
  }
  std::map<std::string, TableMetadata> resulting_tables;
  resulting_tables[output_table_names.front()] =
      data_tables.at(output_table_names.front());
  resulting_tables[output_table_names.front()].sorted_status =
      data_tables.at(input_tables.front()).sorted_status;
  resulting_tables[output_table_names.front()].record_count =
      data_tables.at(input_tables.front()).record_count;
  return std::move(resulting_tables);
}

auto AccelerationModuleSetupInterface::GetWorstCaseNodeCapacity(
    const std::vector<int>& /*min_capacity*/,
    const std::vector<std::string>& /*input_tables*/,
    const std::map<std::string, TableMetadata>& /*data_tables*/,
    QueryOperationType /*next_operation_type*/) -> std::vector<int> {
  return {};
}

auto AccelerationModuleSetupInterface::SetMissingFunctionalCapacity(
    const std::vector<int>& bitstream_capacity,
    std::vector<int>& missing_capacity, const std::vector<int>& node_capacity,
    bool /*is_composed*/) -> bool {
  if (bitstream_capacity.size() != node_capacity.size()) {
    throw std::runtime_error("Capacities don't match!");
  }
  bool is_node_fully_processed = true;
  for (int capacity_parameter_index = 0;
       capacity_parameter_index < bitstream_capacity.size();
       capacity_parameter_index++) {
    missing_capacity.push_back(node_capacity.at(capacity_parameter_index) -
                               bitstream_capacity.at(capacity_parameter_index));
    if (missing_capacity.at(capacity_parameter_index) > 0) {
      is_node_fully_processed = false;
    }
  }
  return is_node_fully_processed;
}

auto AccelerationModuleSetupInterface::UpdateDataTable(
    const std::vector<int>& /*module_capacity*/,
    const std::vector<std::string>& /*input_table_names*/,
    std::map<std::string, TableMetadata>& /*resulting_tables*/) -> bool {
  throw std::runtime_error("Not implemented!");
}

auto AccelerationModuleSetupInterface::InputHasToBeSorted() -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::GetResultingTables(
    const std::map<std::string, TableMetadata>& /*tables*/,
    const std::vector<std::string>& table_names) -> std::vector<std::string> {
  // TODO(Kaspar): Eventually will be removed if a generator module exists.
  if (table_names.empty()) {
    throw std::runtime_error("No input data found");
  }
  return table_names;
}

auto AccelerationModuleSetupInterface::IsReducingData() -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::IsDataSensitive() -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::GetPassthroughInitParameters()
    -> AcceleratedQueryNode {
  AcceleratedQueryNode passthrough_module_node;
  passthrough_module_node.input_streams = {{15, 0, 0, {}, {}}};
  passthrough_module_node.output_streams = {{15, 0, 0, {}, {}}};
  passthrough_module_node.operation_parameters = {};
  return passthrough_module_node;
}
