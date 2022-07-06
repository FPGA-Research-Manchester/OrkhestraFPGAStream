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

#include "merge_sort_setup.hpp"

#include <stdexcept>

#include "logger.hpp"
#include "merge_sort.hpp"
#include "merge_sort_interface.hpp"
#include "query_scheduling_helper.hpp"
#include "stream_parameter_calculator.hpp"

using orkhestrafs::dbmstodspi::MergeSortSetup;

using orkhestrafs::dbmstodspi::MergeSort;
using orkhestrafs::dbmstodspi::MergeSortInterface;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void MergeSortSetup::SetupModule(
    AccelerationModule& acceleration_module,
    const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring merge sort on pos " +
          std::to_string(module_parameters.operation_module_location));
  if (module_parameters.input_streams[0].stream_id != 15) {
    auto index = std::find(module_parameters.composed_module_locations.begin(),
                           module_parameters.composed_module_locations.end(),
                           module_parameters.operation_module_location) -
                 module_parameters.composed_module_locations.begin();

    bool is_first = index == 0;
    // Calculate how many have been before and the sum of them is the index.
    // Then you can also feed the size of this module to the setup.
    int base_id = 0;
    for (int current_index = 0; current_index < index; current_index++) {
      base_id += module_parameters.operation_parameters.at(1).at(current_index);
    }

    MergeSortSetup::SetupMergeSortModule(
        dynamic_cast<MergeSortInterface&>(acceleration_module),
        module_parameters.input_streams[0].stream_id,
        GetStreamRecordSize(module_parameters.input_streams[0]), base_id,
        is_first, module_parameters.operation_parameters.at(1).at(index));
  } else {
    /*throw std::runtime_error(
        "Can't configure merge sort to passthrough on stream ID");*/
    MergeSortSetup::SetupPassthroughMergeSort(
        dynamic_cast<MergeSortInterface&>(acceleration_module));
  }
}

auto MergeSortSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                  int module_position)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<MergeSort>(memory_manager, module_position);
}

void MergeSortSetup::SetupPassthroughMergeSort(
    MergeSortInterface& merge_sort_module) {
  merge_sort_module.SetStreamParams(15, 1);
  // merge_sort_module.StartPrefetchingData(0, false);
}

void MergeSortSetup::SetupMergeSortModule(MergeSortInterface& merge_sort_module,
                                          int stream_id, int record_size,
                                          int base_channel_id, bool is_first, int module_size) {
  int chunks_per_record =
      StreamParameterCalculator::CalculateChunksPerRecord(record_size);

  if (stream_id != 0) {
    throw std::runtime_error(
        "The module doesn't support running other streams!");
  }

  merge_sort_module.SetStreamParams(stream_id, chunks_per_record);

  int buffer_space = (module_size / 32) * 1024;
  // TODO(Kaspar): Remove hardcoded parameters
  // int sort_buffer_size = CalculateSortBufferSize(4096, 128,
  // chunks_per_record);
      int sort_buffer_size =
          CalculateSortBufferSize(buffer_space, module_size, chunks_per_record);
  int record_count_per_fetch =
      CalculateRecordCountPerFetch(sort_buffer_size, record_size);

  merge_sort_module.SetBufferSize(sort_buffer_size);
  merge_sort_module.SetRecordCountPerFetch(record_count_per_fetch);
  merge_sort_module.SetFetchCount(sort_buffer_size / record_count_per_fetch);
  merge_sort_module.SetFetchOffset(sort_buffer_size % record_count_per_fetch);

  merge_sort_module.StartPrefetchingData(base_channel_id, !is_first);
}

auto MergeSortSetup::CalculateSortBufferSize(int buffer_space,
                                             int channel_count,
                                             int chunks_per_record) -> int {
  int internal_logic_buffer_reserve = 240;
  for (int i = 32; i <= channel_count; i *= 2) {
    internal_logic_buffer_reserve += i * 2;
  }
  int max_buffered_record_count = buffer_space / chunks_per_record -
                                  16;  // -16 for records in the pipelines.

  return std::min(buffer_space / channel_count,
                  (max_buffered_record_count - internal_logic_buffer_reserve) /
                      channel_count);
}

auto MergeSortSetup::CalculateRecordCountPerFetch(int sort_buffer_size,
                                                  int record_size) -> int {
  // record_count_per_fetch should be twice as small as sort_buffer_size
  int potential_record_count = sort_buffer_size / 2;
  while (!PotentialRecordCountIsValid(potential_record_count, record_size)) {
    potential_record_count--;
  }
  if (potential_record_count == 0) {
    throw std::runtime_error("Records are too big for sorting!");
  }
  return potential_record_count;
}

auto MergeSortSetup::PotentialRecordCountIsValid(int potential_record_count,
                                                 int record_size) -> bool {
  switch (potential_record_count % 4) {
    case 1:
    case 3:
      return record_size % 4 == 0;
    case 2:
      return record_size % 2 == 0;
    case 0:
      return true;
    default:
      throw std::runtime_error("Something went wrong!");
  }
}
auto MergeSortSetup::IsMultiChannelStream(bool is_input_stream,
                                          int stream_index) -> bool {
  return is_input_stream && stream_index == 0;
}
auto MergeSortSetup::GetMultiChannelParams(
    bool is_input, int /*stream_index*/,
    std::vector<std::vector<int>> operation_parameters)
    -> std::pair<int, std::vector<int>> {
  if (!is_input) {
    return {-1, {}};
  }
  int max_channel_count = 0;
  // Assuming all modules have the same records_per_channel
  std::vector<int> records_per_channel;
  int module_count = operation_parameters.at(0).at(0);
  int parameter_offset = operation_parameters.at(0).at(1);
  for (int module_index = 0; module_index < module_count; module_index++) {
    max_channel_count += static_cast<int>(
        operation_parameters.at(module_index * parameter_offset + 1).size());
    records_per_channel.reserve(
        records_per_channel.size() +
        operation_parameters.at(module_index * parameter_offset + 1).size());
    records_per_channel.insert(
        records_per_channel.end(),
        operation_parameters.at(module_index * parameter_offset + 1).begin(),
        operation_parameters.at(module_index * parameter_offset + 1).end());
  }
  return {max_channel_count, records_per_channel};
}

auto MergeSortSetup::GetMinSortingRequirementsForTable(
    const TableMetadata& table_data) -> std::vector<int> {
  if (table_data.sorted_status.empty()) {
    /*return {table_data.record_count};*/
    throw std::runtime_error("Incorrect sorted meta data");
  }
  if (QuerySchedulingHelper::IsTableSorted(table_data)) {
    return {0};
  }
  int required_capacity = 0;
  for (int i = 3; i < table_data.sorted_status.size(); i += 4) {
    required_capacity += table_data.sorted_status.at(i);
  }
  return {required_capacity};
}

auto MergeSortSetup::IsDataSensitive() -> bool { return true; }
// TODO: Just a weird quirk - Make it more logical later!
// auto MergeSortSetup::IsSortingInputTable() -> bool { return false; }

auto MergeSortSetup::SetMissingFunctionalCapacity(
    const std::vector<int>& bitstream_capacity,
    std::vector<int>& missing_capacity, const std::vector<int>& node_capacity,
    bool is_composed) -> bool {
  if (bitstream_capacity.size() != 1 || node_capacity.size() != 1) {
    throw std::runtime_error("Incorrect capacity values for merge sorter!");
  }
  int missing_capacity_value =
      node_capacity.front() - (bitstream_capacity.front() + int(is_composed));
  if (missing_capacity_value > 0) {
    missing_capacity.push_back(++missing_capacity_value);
    return false;
  } else {
    missing_capacity.push_back(missing_capacity_value);
    return true;
  }
}
