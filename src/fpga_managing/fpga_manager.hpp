#pragma once
#include <cstdint>
#include <vector>
#include "dma.hpp"

class FPGAManager {
 public:
  void SetupQueryAcceleration(
      volatile uint32_t* input_memory_address,
      volatile uint32_t* output_memory_address, int record_size,
      int record_count);
  auto RunQueryAcceleration() -> std::vector<int>;

  explicit FPGAManager(volatile uint32_t* configuration_memory_address)
      : dma_engine_(configuration_memory_address),
        configuration_memory_address_(configuration_memory_address){};

 private:
  const static int kMaxStreamAmount = 16;
  bool input_stream_active_[kMaxStreamAmount] = {false};
  bool output_stream_active_[kMaxStreamAmount] = {false};

  volatile uint32_t* configuration_memory_address_;
  DMA dma_engine_;

  void FindActiveStreams(std::vector<int>& active_input_stream_ids,
                         std::vector<int>& active_output_stream_ids);
  void WaitForStreamsToFinish();
  auto GetResultingStreamSizes(const std::vector<int>& active_input_stream_ids,
                               const std::vector<int>& active_output_stream_ids)
      -> std::vector<int>;
};
