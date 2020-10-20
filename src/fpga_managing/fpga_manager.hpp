#pragma once
#include <cstdint>
#include <vector>
#include "dma.hpp"

#include "fos/cynq.h"

class FPGAManager {
 public:
  void SetupQueryAcceleration(volatile uint32_t* input_memory_address,
                              volatile uint32_t* output_memory_address,
                              int record_size, int record_count);
  auto RunQueryAcceleration() -> std::vector<int>;

  explicit FPGAManager(StaticAccelInst* acceleration_instance)
      : dma_engine_(acceleration_instance),
        acceleration_instance_(acceleration_instance){};

 private:
  const static int kMaxStreamAmount = 16;
  bool input_stream_active_[kMaxStreamAmount] = {false};
  bool output_stream_active_[kMaxStreamAmount] = {false};

  StaticAccelInst* acceleration_instance_;
  DMA dma_engine_;

  void FindActiveStreams(std::vector<int>& active_input_stream_ids,
                         std::vector<int>& active_output_stream_ids);
  void WaitForStreamsToFinish();
  auto GetResultingStreamSizes(const std::vector<int>& active_input_stream_ids,
                               const std::vector<int>& active_output_stream_ids)
      -> std::vector<int>;
};
