#include "stream_parameter_calculator.hpp"
#include "query_acceleration_constants.hpp"

void StreamParameterCalculator::CalculateDMAStreamSetupData(
    DMASetupData& stream_setup_data, const int& max_chunk_size,
    const int& max_ddr_burst_size, const int& max_ddr_size_per_cycle,
    const volatile uint32_t* data_address, const int record_size) {
  stream_setup_data.chunks_per_record =
      (record_size + max_chunk_size - 1) / max_chunk_size;  // ceil

  // Temporarily for now.
  for (int i = 0; i < query_acceleration_constants::kDatapathLength; i++) {
    stream_setup_data.record_chunk_ids.emplace_back(
        i, i % stream_setup_data.chunks_per_record);
  }

  stream_setup_data.records_per_ddr_burst = FindMinViableRecordsPerDDRBurst(
      max_ddr_burst_size, record_size);

  // ceil (recordSize * records_per_ddr_burst) / maxDDRSizePerCycle
  stream_setup_data.ddr_burst_length =
      ((record_size * stream_setup_data.records_per_ddr_burst) +
       max_ddr_size_per_cycle - 1) /
      max_ddr_size_per_cycle;

  // Temporarily for now
  stream_setup_data.buffer_start = 0;
  stream_setup_data.buffer_end = 15;

  stream_setup_data.stream_address = reinterpret_cast<uintptr_t>(data_address);
}

auto StreamParameterCalculator::FindMinViableRecordsPerDDRBurst(
    const int& max_ddr_burst_size, const int& record_size) -> int {
  int records_per_max_burst_size = max_ddr_burst_size / record_size;
  return pow(2, static_cast<int>(log2(records_per_max_burst_size)));
}
