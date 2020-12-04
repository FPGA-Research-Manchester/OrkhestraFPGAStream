#pragma once
#include <cstdint>

#include "dma_setup_data.hpp"

class StreamParameterCalculator
{
 public:
  static void CalculateDMAStreamSetupData(DMASetupData& stream_setup_data,
                                          const int& max_chunk_size,
                                          const int& max_ddr_burst_size,
                                          const int& max_ddr_size_per_cycle,
                                          int record_size);
  static auto FindMinViableRecordsPerDDRBurst(const int& max_ddr_burst_size,
                                              const int& record_size) -> int;
};
