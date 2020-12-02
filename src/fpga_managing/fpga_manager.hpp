#pragma once
#include <cstdint>
#include <vector>
#include "dma.hpp"
#include "memory_manager_interface.hpp"
#include <string>
#include "filter.hpp"
#include "stream_data_parameters.hpp"
#include "ila.hpp"
#include <optional>

class FPGAManager {
 public:
  void SetupQueryAcceleration(
      std::vector<StreamDataParameters> input_streams,
      std::vector<StreamDataParameters> output_streams, bool is_filtering);
  auto RunQueryAcceleration() -> std::vector<int>;

  explicit FPGAManager(MemoryManagerInterface* memory_manager)
      : memory_manager_{memory_manager}, dma_engine_{memory_manager} {};

 private:
  const static int kMaxStreamAmount = 16;
  bool input_stream_active_[kMaxStreamAmount] = {false};
  bool output_stream_active_[kMaxStreamAmount] = {false};

  MemoryManagerInterface* memory_manager_;
  DMA dma_engine_;
  std::optional <ILA> ila_module_;

  void FindActiveStreams(std::vector<int>& active_input_stream_ids,
                         std::vector<int>& active_output_stream_ids);
  void WaitForStreamsToFinish();
  auto GetResultingStreamSizes(const std::vector<int>& active_input_stream_ids,
                               const std::vector<int>& active_output_stream_ids)
      -> std::vector<int>;
  void PrintDebuggingData();
};