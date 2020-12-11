#pragma once
#include <cstdint>
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
#include "stream_data_parameters.hpp"
class DMASetup {
 public:
  static void SetupDMAModule(
      DMAInterface &dma_engine,
      const std::vector<StreamDataParameters> &input_streams,
      const std::vector<StreamDataParameters> &output_streams);

  static void SetupDMAModuleWithMultiStream(
      DMAInterface &dma_engine,
      const std::vector<StreamDataParameters> &input_streams,
      const std::vector<StreamDataParameters> &output_streams);

 private:
  static auto CalculateMultiChannelStreamRecordCountPerChannel(
      int stream_record_count, int max_channel_count, int record_size) -> int;
  static void AddNewStreamDMASetupData(
      DMASetupData &input_stream_setup_data,
      const StreamDataParameters &stream_init_data, int buffer_size,
      std::vector<DMASetupData> &setup_data_for_dma, int current_stream_count);
  static void WriteSetupDataToDMAModule(
      std::vector<DMASetupData> &setup_data_for_dma, DMAInterface &dma_engine);
  static void SetUpDMAIOStreams(DMASetupData &stream_setup_data,
                                DMAInterface &dma_engine);
  static void SetUpDMACrossbars(DMASetupData &stream_setup_data,
                                DMAInterface &dma_engine);
};
