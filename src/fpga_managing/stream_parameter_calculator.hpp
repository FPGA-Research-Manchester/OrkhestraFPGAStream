#pragma once
#include <cstdint>

#include "dma_setup_data.hpp"

class StreamParameterCalculator
{
 public:
  static void CalculateDMAStreamSetupData(DMASetupData& stream_setup_data,
                                          const int record_size);
  static auto FindMinViableRecordsPerDDRBurst(const int record_size) -> int;
  static auto CalculateChunksPerRecord(const int record_size) -> int;
  static auto CalculateDDRBurstLength(const int record_size,
                                      const int records_per_ddr_burst) -> int;
};
