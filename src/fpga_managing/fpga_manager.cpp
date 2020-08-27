#include "fpga_manager.hpp"

#include <iostream>

#include "dma_setup.hpp"
#include "filter.hpp"
#include "filter_setup.hpp"

//#include <unistd.h>

void FPGAManager::SetupQueryAcceleration(
    volatile uint32_t* input_memory_address,
    volatile uint32_t* output_memory_address, int record_size,
    int record_count) {
  int input_stream_id = 0;
  int output_stream_id = 1;
  DMASetup::SetupDMAModule(FPGAManager::dma_engine_, input_memory_address,
                           output_memory_address, record_size, record_count,
                           input_stream_id, output_stream_id);

  Filter filter_module(FPGAManager::configuration_memory_address_, 1);
  FilterSetup::SetupFilterModule(filter_module, input_stream_id,
                                 output_stream_id);
  FPGAManager::input_stream_active_[input_stream_id] = true;
  FPGAManager::output_stream_active_[output_stream_id] = true;
}

auto FPGAManager::RunQueryAcceleration() -> std::vector<int> {
  std::vector<int> active_input_stream_ids;
  std::vector<int> active_output_stream_ids;

  FindActiveStreams(active_input_stream_ids, active_output_stream_ids);

  if (active_input_stream_ids.empty() || active_output_stream_ids.empty()) {
    throw "FPGA does not have active streams!";
  }

  WaitForStreamsToFinish();
  return GetResultingStreamSizes(active_input_stream_ids,
                                 active_output_stream_ids);
}

void FPGAManager::FindActiveStreams(
    std::vector<int>& active_input_stream_ids,
    std::vector<int>& active_output_stream_ids) {
  for (int stream_id = 0; stream_id << FPGAManager::kMaxStreamAmount;
       stream_id++) {
    if (input_stream_active_[stream_id]) {
      active_input_stream_ids.push_back(stream_id);
    }
    if (output_stream_active_[stream_id]) {
      active_output_stream_ids.push_back(stream_id);
    }
  }
}

void FPGAManager::WaitForStreamsToFinish() {
  FPGAManager::dma_engine_.StartInputController(
      FPGAManager::input_stream_active_);
  FPGAManager::dma_engine_.StartOutputController(
      FPGAManager::output_stream_active_);

  while (!(FPGAManager::dma_engine_.IsInputControllerFinished() &&
           FPGAManager::dma_engine_.IsOutputControllerFinished())) {
    std::cout << "Processing..." << std::endl;
    std::cout << "Input:" << FPGAManager::dma_engine_.IsInputControllerFinished()
              << std::endl;
    std::cout << "Output:"
              << FPGAManager::dma_engine_.IsOutputControllerFinished()
              << std::endl;
    // sleep(1);
  }
}

auto FPGAManager::GetResultingStreamSizes(
    const std::vector<int>& active_input_stream_ids,
    const std::vector<int>& active_output_stream_ids) -> std::vector<int> {
  for (auto stream_id : active_input_stream_ids) {
    FPGAManager::input_stream_active_[stream_id] = false;
  }
  std::vector<int> result_sizes;
  for (auto stream_id : active_output_stream_ids) {
    FPGAManager::output_stream_active_[stream_id] = false;
    result_sizes.push_back(dma_engine_.GetOutputControllerStreamSize(stream_id));
  }
  return result_sizes;
}
