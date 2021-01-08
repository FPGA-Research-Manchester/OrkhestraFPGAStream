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
  static void SetUpDMAIOStreams(DMASetupData &stream_setup_data,
                                DMAInterface &dma_engine);
  static void SetUpDMACrossbars(DMASetupData &stream_setup_data,
                                DMAInterface &dma_engine);
};
