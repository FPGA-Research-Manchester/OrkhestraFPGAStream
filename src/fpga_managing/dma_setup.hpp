#pragma once
#include <cstdint>
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
#include "stream_initialisation_data.hpp"
class DMASetup {
 public:
  static void SetupDMAModule(
      DMAInterface &dma_engine,
      std::vector<StreamInitialisationData> input_streams,
      std::vector<StreamInitialisationData> output_streams);

 private:
  static void AddNewStreamDMASetupData(
      DMASetupData &input_stream_setup_data,
      StreamInitialisationData &stream_init_data, const int &max_chunk_size,
      const int &max_ddr_burst_size, const int &max_ddr_size_per_cycle,
      const int &any_chunk, const int &any_position, int buffer_size,
      std::vector<DMASetupData> &setup_data_for_dma);
  static void WriteSetupDataToDMAModule(
      std::vector<DMASetupData>& setup_data_for_dma, DMAInterface& dma_engine);
  static void SetUpDMAIOStreams(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
  static void SetUpDMACrossbars(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
};
