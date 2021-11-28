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

#include "linear_sort_setup.hpp"

#include <stdexcept>

#include "linear_sort.hpp"
#include "linear_sort_interface.hpp"
#include "logger.hpp"
#include "stream_parameter_calculator.hpp"

using orkhestrafs::dbmstodspi::LinearSortSetup;

using orkhestrafs::dbmstodspi::LinearSort;
using orkhestrafs::dbmstodspi::LinearSortInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void LinearSortSetup::SetupModule(
    AccelerationModule& acceleration_module,
    const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring linear sort on pos " +
          std::to_string(module_parameters.operation_module_location));
  LinearSortSetup::SetupLinearSortModule(
      dynamic_cast<LinearSortInterface&>(acceleration_module),
      module_parameters.input_streams[0].stream_id,
      GetStreamRecordSize(module_parameters.input_streams[0]));
}

auto LinearSortSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                   int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<LinearSort>(memory_manager, module_postion);
}

void LinearSortSetup::SetupLinearSortModule(
    LinearSortInterface& linear_sort_module, int stream_id, int record_size) {
  int chunks_per_record =
      StreamParameterCalculator::CalculateChunksPerRecord(record_size);

  linear_sort_module.SetStreamParams(stream_id, chunks_per_record);

  linear_sort_module.StartPrefetchingData();
}

auto LinearSortSetup::GetMinSortingRequirementsForTable(
    const TableMetadata& table_data) -> std::vector<int> {
  return {table_data.record_count};
}

auto LinearSortSetup::GetSortedSequenceWithCapacity(int bitstream_capacity,
                                                    int record_count)
    -> std::vector<SortedSequence> {
  int sequence_count = record_count / bitstream_capacity;
  if (sequence_count == 0) {
    return {{0, record_count}};
  } else {
    std::vector<SortedSequence> new_sorted_sequences;
    for (int sequence_index = 0; sequence_index < sequence_count;
         sequence_index++) {
      new_sorted_sequences.push_back(
          {bitstream_capacity * sequence_index, bitstream_capacity});
      if (sequence_index == sequence_count - 1 &&
          record_count % bitstream_capacity != 0) {
        new_sorted_sequences.push_back({bitstream_capacity * sequence_count,
                                        record_count % bitstream_capacity});
      }
    }
    return new_sorted_sequences;
  }
}

auto LinearSortSetup::GetWorstCaseProcessedTables(
    const std::vector<int>& min_capacity,
    const std::vector<std::string>& input_tables,
    const std::map<std::string, TableMetadata>& data_tables)
    -> std::map<std::string, TableMetadata> {
  if (min_capacity.size() != 1) {
    throw std::runtime_error("Inncorrect capacity values given!");
  }
  std::map<std::string, TableMetadata> resulting_tables;
  for (const auto& table_name : input_tables) {
    auto new_table_name = table_name;
    auto new_table_data = data_tables.at(table_name);
    new_table_data.sorted_status = GetSortedSequenceWithCapacity(
        min_capacity.front(), new_table_data.record_count);
    if (new_table_data.sorted_status.size() != 1) {
      new_table_name += "_half_sorted";
    } else {
      new_table_name += "_fully_sorted";
    }
    resulting_tables.insert({new_table_name, new_table_data});
  }
  return resulting_tables;
}

auto LinearSortSetup::UpdateDataTable(
    const std::vector<int>& module_capacity,
    const std::vector<std::string>& input_table_names,
    const std::map<std::string, TableMetadata>& data_tables,
    std::map<std::string, TableMetadata>& resulting_tables) -> bool {
  if (input_table_names.size() != 1) {
    throw std::runtime_error("Wrong number of tables!");
  }
  if (module_capacity.size() != 1) {
    throw std::runtime_error("Wrong linear sort capacity given!");
  }
  auto current_table = data_tables.at(input_table_names.front());
  auto new_sorted_sequence = GetSortedSequenceWithCapacity(
      module_capacity.front(), current_table.record_count);
  current_table.sorted_status = {new_sorted_sequence};
  resulting_tables.at(input_table_names.front()) = current_table;
  return true;
}
