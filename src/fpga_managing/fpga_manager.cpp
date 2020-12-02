#include "fpga_manager.hpp"

#include <iostream>

#include "dma_setup.hpp"
#include "filter.hpp"
#include "filter_setup.hpp"
#include "ila.hpp"
#include "join.hpp"
#include "join_setup.hpp"

void FPGAManager::SetupQueryAcceleration(
    std::vector<StreamInitialisationData> input_streams,
    std::vector<StreamInitialisationData> output_streams, bool is_filtering) {
  if (ila_module_) {
    ila_module_.value().startAxiILA();
  }
  DMASetup::SetupDMAModule(dma_engine_, input_streams, output_streams);
  for (const auto& stream : input_streams) {
    FPGAManager::input_stream_active_[stream.stream_id] = true;
  }
  FPGAManager::dma_engine_.StartInputController(
      FPGAManager::input_stream_active_);

  if (ila_module_) {
    ila_module_.value().startILAs();
  }
  if (is_filtering) {
    Filter filter_module(memory_manager_, 1);
    FilterSetup::SetupFilterModule(filter_module, input_streams[0].stream_id,
                                   output_streams[0].stream_id);
  } else {
    Join join_module(memory_manager_, 1);
    JoinSetup::SetupJoinModule(join_module, input_streams[0].stream_id,
                               input_streams[1].stream_id,
                               output_streams[0].stream_id);
  }

  for (const auto& stream : output_streams) {
    FPGAManager::output_stream_active_[stream.stream_id] = true;
  }
}

auto FPGAManager::RunQueryAcceleration() -> std::vector<int> {
  std::vector<int> active_input_stream_ids;
  std::vector<int> active_output_stream_ids;

  FindActiveStreams(active_input_stream_ids, active_output_stream_ids);

  if (active_input_stream_ids.empty() || active_output_stream_ids.empty()) {
    throw "FPGA does not have active streams!";
  }

  WaitForStreamsToFinish();
  PrintDebuggingData();
  return GetResultingStreamSizes(active_input_stream_ids,
                                 active_output_stream_ids);
}

void FPGAManager::FindActiveStreams(
    std::vector<int>& active_input_stream_ids,
    std::vector<int>& active_output_stream_ids) {
  for (int stream_id = 0; stream_id < FPGAManager::kMaxStreamAmount;
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
  FPGAManager::dma_engine_.StartOutputController(
      FPGAManager::output_stream_active_);

#ifdef _FPGA_AVAILABLE
  while (!(FPGAManager::dma_engine_.IsInputControllerFinished() &&
           FPGAManager::dma_engine_.IsOutputControllerFinished())) {
    std::cout << "Processing..." << std::endl;
    std::cout << "Input:"
              << FPGAManager::dma_engine_.IsInputControllerFinished()
              << std::endl;
    std::cout << "Output:"
              << FPGAManager::dma_engine_.IsOutputControllerFinished()
              << std::endl;
  }
#endif
}

auto FPGAManager::GetResultingStreamSizes(
    const std::vector<int>& active_input_stream_ids,
    const std::vector<int>& active_output_stream_ids) -> std::vector<int> {
  for (auto stream_id : active_input_stream_ids) {
    FPGAManager::input_stream_active_[stream_id] = false;
  }
  std::vector<int> result_sizes(16, 0);
  for (auto stream_id : active_output_stream_ids) {
    FPGAManager::output_stream_active_[stream_id] = false;
    result_sizes[stream_id] =
        dma_engine_.GetOutputControllerStreamSize(stream_id);
  }
  return result_sizes;
}

void FPGAManager::PrintDebuggingData() {
#ifdef _FPGA_AVAILABLE
  std::cout << "Runtime: " << std::dec << FPGAManager::dma_engine_.GetRuntime()
            << std::endl;
  std::cout << "ValidReadCount:" << FPGAManager::dma_engine_.GetRuntime()
            << std::endl;
  std::cout << "ValidWriteCount:" << FPGAManager::dma_engine_.GetRuntime()
            << std::endl;
  if (FPGAManager::ila_module_) {
    std::cout << "======================================================ILA 0 "
                 "DATA ======================================================="
              << std::endl;
    FPGAManager::ila_module_.value().PrintILAData(0, 2048);
    std::cout << "======================================================ILA 1 "
                 "DATA ======================================================="
              << std::endl;
    FPGAManager::ila_module_.value().PrintILAData(1, 2048);
    std::cout << "======================================================ILA 2 "
                 "DATA ======================================================="
              << std::endl;
    FPGAManager::ila_module_.value().PrintAxiILAData(4096);
  }
#endif
}
