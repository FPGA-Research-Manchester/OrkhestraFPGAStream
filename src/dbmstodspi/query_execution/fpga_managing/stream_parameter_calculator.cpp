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

#include "stream_parameter_calculator.hpp"

#include <algorithm>
#include <cmath>

#include "merge_sort_setup.hpp"
#include "query_acceleration_constants.hpp"

using orkhestrafs::dbmstodspi::StreamParameterCalculator;

void StreamParameterCalculator::CalculateRecordCountPerFetchMultiStream(
    DMASetupData& stream_setup_data, const int record_size,
    const int smallest_module_size, const int record_size_after_crossbar) {
  int buffer_space = (smallest_module_size / 32) * 1024;
  int sort_buffer_size = MergeSortSetup::CalculateSortBufferSize(
      buffer_space, smallest_module_size, stream_setup_data.chunks_per_record);
  stream_setup_data.records_per_ddr_burst =
      MergeSortSetup::CalculateRecordCountPerFetch(
          sort_buffer_size, record_size, record_size_after_crossbar);
}

void StreamParameterCalculator::CalculateDMAStreamSetupData(
    DMASetupData& stream_setup_data, const int record_size,
    bool is_multichannel_stream, const int smallest_module_size,
    const int record_size_after_crossbar) {
  // Temporarily for now. Possibly wrong for output which is reduced in chunks!
  for (int i = 0; i < query_acceleration_constants::kDatapathLength; i++) {
    stream_setup_data.record_chunk_ids.emplace_back(
        i, i % FindNextPowerOfTwo(stream_setup_data.chunks_per_record));
  }

  if (is_multichannel_stream) {
    CalculateRecordCountPerFetchMultiStream(stream_setup_data, record_size,
                                            smallest_module_size,
                                            record_size_after_crossbar);
  } else {
    if (stream_setup_data.is_input_stream) {
      stream_setup_data.records_per_ddr_burst = FindMinViableRecordsPerDDRBurst(
          std::max(record_size_after_crossbar, record_size));
      /*stream_setup_data.records_per_ddr_burst =
          FindMinViableRecordsPerDDRBurst(record_size);*/
    } else {
      // chunks_per_record can't be bigger than 32.
      stream_setup_data.records_per_ddr_burst =
          query_acceleration_constants::kMaxRecordsPerDDRBurst /
          FindNextPowerOfTwo(stream_setup_data.chunks_per_record);
    }
  }

  // ceil (recordSize * records_per_ddr_burst) / maxDDRSizePerCycle
  /*stream_setup_data.ddr_burst_length = CalculateDDRBurstLength(std::max(record_size_after_crossbar, record_size),
                              stream_setup_data.records_per_ddr_burst);*/
  stream_setup_data.ddr_burst_length = CalculateDDRBurstLength(
      record_size, stream_setup_data.records_per_ddr_burst);
}

auto StreamParameterCalculator::FindMinViableRecordsPerDDRBurst(
    const int record_size) -> int {
  int records_per_max_burst_size =
      query_acceleration_constants::kDdrBurstSize / record_size;
  return std::min(query_acceleration_constants::kMaxRecordsPerDDRBurst,
                  static_cast<int>(pow(
                      2, static_cast<int>(log2(records_per_max_burst_size)))));
}

auto StreamParameterCalculator::CalculateChunksPerRecord(const int record_size)
    -> int {
  return (record_size + query_acceleration_constants::kDatapathWidth - 1) /
         query_acceleration_constants::kDatapathWidth;
}

auto StreamParameterCalculator::CalculateDDRBurstLength(
    const int record_size, const int records_per_ddr_burst) -> int {
  return ((record_size * records_per_ddr_burst) +
          query_acceleration_constants::kDdrSizePerCycle - 1) /
         query_acceleration_constants::kDdrSizePerCycle;
}

auto StreamParameterCalculator::FindNextPowerOfTwo(const int& value) -> int {
  return pow(2, static_cast<int>(ceil(log2(value))));
}
