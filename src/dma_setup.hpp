#pragma once
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
class DMASetup {
 public:
  static void SetupDMAModule(DMAInterface& dma_engine,
                             std::vector<int>& db_data, int record_size,
                             int record_count, int input_stream_id,
                             int output_stream_id);

 private:
  static void WriteSetupDataToDMAModule(
      std::vector<DMASetupData>& setup_data_for_dma, DMAInterface& dma_engine);
  static void SetUpDMAIOStreams(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
  static void SetUpDMACrossbars(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
  static void CalculateDMAStreamSetupData(DMASetupData& stream_setup_data,
                                          const int& max_chunk_size,
                                          const int& max_ddr_burst_size,
                                          const int& max_ddr_size_per_cycle,
                                          std::vector<int>& db_data,
                                          int record_size);
};
