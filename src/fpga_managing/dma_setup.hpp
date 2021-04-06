#pragma once
#include <cstdint>
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
#include "stream_data_parameters.hpp"
/**
 * Class to setup DMA configuration data and write the data into the DMA
 * registers.
 */
class DMASetup {
 public:
  /**
   * Based on the input stream parameters calculate DMA setup data and then
   *  write the configuration data to the memory mapped registers.
   * @param dma_engine The modules where the information about memory mapped
   *  registers is.
   * @param streams Vector of definition data for all of the input or output
   *  streams.
   * @param is_input_stream Boolean saying if the given streams are input or
   *  output streams.
   */
  static void SetupDMAModule(DMAInterface &dma_engine,
                             const std::vector<StreamDataParameters> &streams,
                             bool is_input_stream);

 private:
  static auto CalculateMultiChannelStreamRecordCountPerChannel(
      int stream_record_count, int max_channel_count, int record_size) -> int;
  static void SetUpDMAIOStream(const DMASetupData &stream_setup_data,
                               DMAInterface &dma_engine);
  static void SetUpDMACrossbarsForStream(const DMASetupData &stream_setup_data,
                                         DMAInterface &dma_engine);
  static void SetSingleChannelSetupData(
      DMASetupData &stream_setup_data, const bool &is_input_stream,
      const StreamDataParameters &stream_init_data);
  static void SetMultiChannelSetupData(
      const StreamDataParameters &stream_init_data,
      DMASetupData &stream_setup_data);
  static void AllocateStreamBuffers(DMASetupData &stream_setup_data,
                                    const int &buffer_size,
                                    int current_stream_count);
};
