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

#include <cmath>
#include <stdexcept>

#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

using namespace dbmstodspi::fpga_managing;

void MergeSortSetup::SetupMergeSortModule(
    modules::MergeSortInterface& merge_sort_module, int stream_id,
    int record_size, int base_channel_id, bool is_first) {
  int chunks_per_record =
      StreamParameterCalculator::CalculateChunksPerRecord(record_size);

  if (stream_id != 0) {
    throw std::runtime_error(
        "The module doesn't support running other streams!");
  }

  merge_sort_module.SetStreamParams(stream_id, chunks_per_record);

  int sort_buffer_size = CalculateSortBufferSize(2048, 64, chunks_per_record);
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

  return std::min(16,
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
