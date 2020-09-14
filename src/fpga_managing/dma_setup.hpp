#pragma once
#include <cstdint>
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
class DMASetup {
 public:
  static void SetupDMAModule(DMAInterface& dma_engine,
                             volatile uint32_t* input_memory_address,
                             volatile uint32_t* output_memory_address,
                             int record_size, int record_count,
                             int input_stream_id, int output_stream_id);

 private:
  static void WriteSetupDataToDMAModule(
      std::vector<DMASetupData>& setup_data_for_dma, DMAInterface& dma_engine);
  static void SetUpDMAIOStreams(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
  static void SetUpDMACrossbars(DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine);
};
