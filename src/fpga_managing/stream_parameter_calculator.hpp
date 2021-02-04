#pragma once
#include <cstdint>

#include "dma_setup_data.hpp"

class StreamParameterCalculator {
 public:
  static void CalculateDMAStreamSetupData(DMASetupData& stream_setup_data,
                                          int record_size,
                                          bool is_multichannel_stream);
  static auto FindMinViableRecordsPerDDRBurst(int record_size) -> int;
  static auto CalculateChunksPerRecord(int record_size) -> int;
  static auto CalculateDDRBurstLength(int record_size,
                                      int records_per_ddr_burst) -> int;

  static auto FindNextPowerOfTwo(const int& value) -> int;
};
