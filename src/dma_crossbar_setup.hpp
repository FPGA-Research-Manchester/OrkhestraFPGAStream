#pragma once
#include <queue>

#include "dma_setup_data.hpp"
class DMACrossbarSetup {
 public:
  static void FindInputCrossbarSetupData(const int& any_chunk,
                                         const int& any_position,
                                         DMASetupData& input_stream_setup_data);
  static void FindOutputCrossbarSetupData(
      const int& any_chunk, const int& any_position,
      DMASetupData& output_stream_setup_data);

 private:
  static void CalculateInterfaceToBufferSetupConfig(
      std::queue<int>& source_chunks, std::queue<int>& target_positions,
      const int& any_chunk, const int& any_position);
  static void CalculateBufferToInterfaceSetupConfig(
      std::queue<int>& source_chunks, std::queue<int>& target_positions,
      const int& any_chunk, const int& any_position);
  static void SetCrossbarSetupDataForStream(std::queue<int>& source_chunks,
                                            std::queue<int>& target_positions,
                                            DMASetupData& stream_setup_data);
};
