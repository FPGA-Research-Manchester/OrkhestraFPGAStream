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
    const std::map<std::string, TableMetadata>& data_tables)
    -> std::map<std::string, TableMetadata> {
  std::map<std::string, TableMetadata> resulting_tables;
  for (const auto& input_table_name : input_tables) {
    resulting_tables[input_table_name] = data_tables.at(input_table_name);
    /*resulting_tables.insert(
        {input_table_name, data_tables.at(input_table_name)});*/
  }
  return resulting_tables;
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
  passthrough_module_node.input_streams = {{15, 0, 0, nullptr, {}}};
  passthrough_module_node.output_streams = {{15, 0, 0, nullptr, {}}};
  passthrough_module_node.operation_parameters = {};
  return passthrough_module_node;
}
