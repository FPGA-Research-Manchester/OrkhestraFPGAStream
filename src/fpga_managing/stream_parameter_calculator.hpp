#pragma once
#include <cstdint>

#include "dma_setup_data.hpp"

/**
 * Helper class to calculate the DMA configuration parameters based on stream
 * data.
 */
class StreamParameterCalculator {
 public:
  /**
   * Calculate required parameters for setting up the DMA based on the given
   *  stream
   * @param stream_setup_data Where the results will be stored.
   * @param record_size How many integers worth of data there is in each record.
   * @param is_multichannel_stream Is the given stream a multi-channel stream.
   */
  static void CalculateDMAStreamSetupData(DMASetupData& stream_setup_data,
                                          int record_size,
                                          bool is_multichannel_stream);
  /**
   * Find if it would be more efficient to transfer less than max amount of
   *  records each DDR burst.
   * @param record_size How many integers worth of data there is in each record.
   * @return How many records should be transferred each DDR burst.
   */
  static auto FindMinViableRecordsPerDDRBurst(int record_size) -> int;
  /**
   * Calculate how many chunks are required for each record.
   * @param record_size How many integers worth of data each record has.
   * @return How many chunks are required to hold each record.
   */
  static auto CalculateChunksPerRecord(int record_size) -> int;
  /**
   * Calculate how many clock cycles the DDR burst should be.
   * @param record_size How many integers worth of data each record has.
   * @param records_per_ddr_burst How many records are going to get transferred
   *  each DDR burst.
   * @return Amount of required clock cycles for each DDR burst.
   */
  static auto CalculateDDRBurstLength(int record_size,
                                      int records_per_ddr_burst) -> int;
  /**
   * Find the next power of two value.
   * @param value Input integer
   * @return The next integer which is a power of two.
   */
  static auto FindNextPowerOfTwo(const int& value) -> int;
};
