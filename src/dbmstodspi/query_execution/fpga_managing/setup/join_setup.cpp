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

#include "join_setup.hpp"

#include <algorithm>

#include "join.hpp"
#include "join_interface.hpp"
#include "logger.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

using orkhestrafs::dbmstodspi::JoinSetup;

using orkhestrafs::dbmstodspi::Join;
using orkhestrafs::dbmstodspi::JoinInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void JoinSetup::SetupModule(AccelerationModule& acceleration_module,
                            const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring join on pos " +
          std::to_string(module_parameters.operation_module_location));
  if (module_parameters.input_streams[0].stream_id != 15) {
    JoinSetup::SetupJoinModule(
        dynamic_cast<JoinInterface&>(acceleration_module),
        module_parameters.input_streams[0].stream_id,
        GetStreamRecordSize(module_parameters.input_streams[0]),
        module_parameters.input_streams[1].stream_id,
        GetStreamRecordSize(module_parameters.input_streams[1]),
        module_parameters.output_streams[0].stream_id,
        module_parameters.output_streams[0].input_chunks_per_record,
        module_parameters.operation_parameters.at(0).at(0));
  } else {
    JoinSetup::SetupPassthroughJoin(
        dynamic_cast<JoinInterface&>(acceleration_module));
  }
}

void JoinSetup::SetupPassthroughJoin(JoinInterface& join_module) {
  join_module.Reset();
  join_module.DefineOutputStream(1, 15, 15, 15);
  join_module.StartPrefetchingData();
}

auto JoinSetup::CreateModule(MemoryManagerInterface* memory_manager,
                             int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<Join>(memory_manager, module_postion);
}

void JoinSetup::SetupJoinModule(JoinInterface& join_module,
                                int first_input_stream_id,
                                int first_input_record_size,
                                int second_input_stream_id,
                                int second_input_record_size,
                                int output_stream_id,
                                int output_chunks_per_record, int shift_size) {
  join_module.Reset();
  join_module.DefineOutputStream(output_chunks_per_record,
                                 first_input_stream_id, second_input_stream_id,
                                 output_stream_id);
  join_module.SetFirstInputStreamChunkCount(
      StreamParameterCalculator::CalculateChunksPerRecord(
          first_input_record_size));
  join_module.SetSecondInputStreamChunkCount(
      StreamParameterCalculator::CalculateChunksPerRecord(
          second_input_record_size));

  SetupTimeMultiplexer(join_module, first_input_record_size,
                       second_input_record_size, shift_size);

  join_module.StartPrefetchingData();
}

void JoinSetup::SetupTimeMultiplexer(JoinInterface& join_module,
                                     int first_stream_size,
                                     int second_stream_size, int shift_size) {
  int output_chunk_id = 0;
  int data_position = query_acceleration_constants::kDatapathWidth - 1;
  for (int first_stream_element_count = 0;
       first_stream_element_count < first_stream_size;
       first_stream_element_count++) {
    join_module.SelectOutputDataElement(
        output_chunk_id,
        first_stream_element_count /
            query_acceleration_constants::kDatapathWidth,
        data_position, false);
    data_position--;
    if (data_position == -1) {
      data_position = query_acceleration_constants::kDatapathWidth - 1;
      output_chunk_id++;
    }
  }

  for (int second_stream_element_count = 0;
       second_stream_element_count < second_stream_size;
       second_stream_element_count++) {
    if (second_stream_element_count >= shift_size) {
      join_module.SelectOutputDataElement(
          output_chunk_id,
          second_stream_element_count /
              query_acceleration_constants::kDatapathWidth,
          data_position, true);
      data_position--;
      if (data_position == -1) {
        data_position = query_acceleration_constants::kDatapathWidth - 1;
        output_chunk_id++;
      }
    }
  }
}

// TODO(Kaspar): Should also check the node parameter to know what does the
// crossbar do. Should also preserve the sorted status somehow.
auto JoinSetup::GetWorstCaseProcessedTables(
    const std::vector<int>& /*min_capacity*/,
    const std::vector<std::string>& input_tables,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::vector<std::string>& output_table_names)
    -> std::map<std::string, TableMetadata> {
  if (input_tables.size() != 2 || output_table_names.size() != 1) {
    throw std::runtime_error("Unsupporded table counts at join preprocessing!");
  }
  std::map<std::string, TableMetadata> resulting_tables;
  resulting_tables[output_table_names.front()] =
      data_tables.at(output_table_names.front());
  // Record count is an estimation
  resulting_tables[output_table_names.front()].record_count =
      std::min(data_tables.at(input_tables.front()).record_count,
               data_tables.at(input_tables.back()).record_count);
  resulting_tables[output_table_names.front()].sorted_status = {
      0, resulting_tables[output_table_names.front()].record_count - 1,
      resulting_tables[output_table_names.front()].record_count, 1};
  return std::move(resulting_tables);
}

auto JoinSetup::InputHasToBeSorted() -> bool { return true; }

// Just pick the largest table name for now.
auto JoinSetup::GetResultingTables(
    const std::map<std::string, TableMetadata>& tables,
    const std::vector<std::string>& table_names) -> std::vector<std::string> {
  std::string max_table_name;
  int max_size = 0;
  for (const auto& table_name : table_names) {
    if (tables.at(table_name).record_count >= max_size) {
      max_table_name = table_name;
      max_size = tables.at(table_name).record_count;
    }
  }
  return {max_table_name};
}