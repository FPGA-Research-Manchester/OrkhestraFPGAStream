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

 private:
  static void AddNewStreamDMASetupData(
      DMASetupData &input_stream_setup_data,
      const StreamDataParameters &stream_init_data, const int &max_chunk_size,
      const int &max_ddr_burst_size, const int &max_ddr_size_per_cycle,
      const int &any_chunk, const int &any_position, int buffer_size,
      std::vector<DMASetupData> &setup_data_for_dma, int current_stream_count);
  static void WriteSetupDataToDMAModule(
      std::vector<DMASetupData>& setup_data_for_dma, DMAInterface& dma_engine);
  static void SetUpDMAIOStreams(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
  static void SetUpDMACrossbars(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
};
