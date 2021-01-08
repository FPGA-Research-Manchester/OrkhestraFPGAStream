#include "stream_parameter_calculator.hpp"
#include "query_acceleration_constants.hpp"
#include "merge_sort_setup.hpp"

#include <cmath>
#include <algorithm>

void StreamParameterCalculator::CalculateDMAStreamSetupData(
    DMASetupData& stream_setup_data, 
    const int record_size, bool is_multichannel_stream) {
  stream_setup_data.chunks_per_record = CalculateChunksPerRecord(record_size);

  // Temporarily for now.
  for (int i = 0; i < query_acceleration_constants::kDatapathLength; i++) {
    stream_setup_data.record_chunk_ids.emplace_back(
        i, i % stream_setup_data.chunks_per_record);
  }

  if (is_multichannel_stream) {
    int sort_buffer_size = MergeSortSetup::CalculateSortBufferSize(
        2048, 64, stream_setup_data.chunks_per_record);
    stream_setup_data.records_per_ddr_burst =
        MergeSortSetup::CalculateRecordCountPerFetch(sort_buffer_size,
                                                     record_size);
  }else {
    stream_setup_data.records_per_ddr_burst =
        FindMinViableRecordsPerDDRBurst(record_size);
  }
  
  // ceil (recordSize * records_per_ddr_burst) / maxDDRSizePerCycle
  stream_setup_data.ddr_burst_length = CalculateDDRBurstLength(
      record_size, stream_setup_data.records_per_ddr_burst);
}

auto StreamParameterCalculator::FindMinViableRecordsPerDDRBurst(const int record_size) -> int {
  int records_per_max_burst_size =
      query_acceleration_constants::kDdrBurstSize / record_size;
  return std::min(query_acceleration_constants::kMaxRecordsPerDDRBurst,
                  static_cast<int>(pow(2, static_cast<int>(log2(records_per_max_burst_size)))));
}

auto StreamParameterCalculator::CalculateChunksPerRecord(
    const int record_size) -> int{
  return (record_size + query_acceleration_constants::kDatapathWidth - 1) /
         query_acceleration_constants::kDatapathWidth;
}

auto StreamParameterCalculator::CalculateDDRBurstLength(
    const int record_size, const int records_per_ddr_burst) -> int {
  return ((record_size * records_per_ddr_burst) + query_acceleration_constants::kDdrSizePerCycle -1) / query_acceleration_constants::kDdrSizePerCycle;
}
