#pragma once
#include <cstdint>
#include <utility>
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
#include "stream_data_parameters.hpp"
class DMASetup {
 public:
  static void SetupDMAModule(
      DMAInterface &dma_engine,
      const std::vector<std::pair<StreamDataParameters, bool>> &streams,
      const bool is_input_stream);

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
      const int &max_channel_count, DMASetupData &stream_setup_data);
  static void AllocateStreamBuffers(DMASetupData &stream_setup_data,
                                    const int &buffer_size,
                                    int current_stream_count);
};
