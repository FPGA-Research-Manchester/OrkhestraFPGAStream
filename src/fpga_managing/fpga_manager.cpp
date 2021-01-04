#include "fpga_manager.hpp"

#include <iostream>

#include "dma_setup.hpp"
#include "filter.hpp"
#include "filter_setup.hpp"
#include "ila.hpp"
#include "join.hpp"
#include "join_setup.hpp"
#include "merge_sort.hpp"
#include "merge_sort_setup.hpp"
#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"

// Eventually the idea would be to add operation type to StreamDataParameters.
// Then the modules and streams can be set up accordingly.
void FPGAManager::SetupQueryAcceleration(
    const std::vector<AcceleratedQueryNode>& query_nodes) {
  dma_engine_.GlobalReset();

  // ila_module_ = std::make_optional (ILA(memory_manager_));
  /*if (ila_module_) {
    ila_module_.value().startAxiILA();
  }*/

  for (const auto& query_node : query_nodes) {
    // Input and output should be setup separately such that these two booleans
    // don't have to be ANDed.
    if (!query_node.is_input_intermediate &&
        !query_node.is_output_intermediate) {
      if (query_node.operation_type !=
          operation_types::QueryOperation::kMergeSort) {
        DMASetup::SetupDMAModule(dma_engine_, query_node.input_streams,
                                 query_node.output_streams);
      } else {
        DMASetup::SetupDMAModuleWithMultiStream(
            dma_engine_, query_node.input_streams, query_node.output_streams);
      }
    }
  }

  for (const auto& query_node : query_nodes) {
    if (!query_node.is_input_intermediate &&
        !query_node.is_output_intermediate) {
      for (const auto& stream : query_node.input_streams) {
        FPGAManager::input_streams_active_status_[stream.stream_id] = true;
      }
    }
  }
  FPGAManager::dma_engine_.StartInputController(
      FPGAManager::input_streams_active_status_);

  if (ila_module_) {
    ila_module_.value().startILAs();
  }

  for (const auto& query_node : query_nodes) {
    switch (query_node.operation_type) {
      case operation_types::QueryOperation::kFilter: {
        Filter filter_module(memory_manager_, 1);
        FilterSetup::SetupFilterModule(filter_module,
                                       query_node.input_streams[0].stream_id,
                                       query_node.output_streams[0].stream_id);
        break;
      }
      case operation_types::QueryOperation::kJoin: {
        Join join_module(memory_manager_, 1);
        JoinSetup::SetupJoinModule(join_module,
                                   query_node.input_streams[0].stream_id,
                                   query_node.input_streams[1].stream_id,
                                   query_node.output_streams[0].stream_id);
        break;
      }
      case operation_types::QueryOperation::kMergeSort: {
        MergeSort merge_sort_module(memory_manager_, 1);
        MergeSortSetup::SetupMergeSortModule(
            merge_sort_module, query_node.input_streams[0].stream_id,
            query_node.input_streams[0].stream_record_size, 0, true);
        MergeSort merge_sort_module_last(memory_manager_, 2);
        MergeSortSetup::SetupMergeSortModule(
            merge_sort_module_last, query_node.input_streams[0].stream_id,
            query_node.input_streams[0].stream_record_size, 64, false);
        break;
      }
    }
  }

  for (const auto& query_node : query_nodes) {
    if (!query_node.is_input_intermediate &&
        !query_node.is_output_intermediate) {
      for (const auto& stream : query_node.output_streams) {
        FPGAManager::output_streams_active_status_[stream.stream_id] = true;
      }
    }
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
  // PrintDebuggingData();
  return GetResultingStreamSizes(active_input_stream_ids,
                                 active_output_stream_ids);
}

void FPGAManager::FindActiveStreams(
    std::vector<int>& active_input_stream_ids,
    std::vector<int>& active_output_stream_ids) {
  for (int stream_id = 0; stream_id < FPGAManager::kMaxStreamAmount;
       stream_id++) {
    if (input_streams_active_status_[stream_id]) {
      active_input_stream_ids.push_back(stream_id);
    }
    if (output_streams_active_status_[stream_id]) {
      active_output_stream_ids.push_back(stream_id);
    }
  }
}

void FPGAManager::WaitForStreamsToFinish() {
  FPGAManager::dma_engine_.StartOutputController(
      FPGAManager::output_streams_active_status_);

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
    FPGAManager::input_streams_active_status_[stream_id] = false;
  }
  std::vector<int> result_sizes(16, 0);
  for (auto stream_id : active_output_stream_ids) {
    FPGAManager::output_streams_active_status_[stream_id] = false;
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
    FPGAManager::ila_module_.value().PrintDMAILAData(2048);
  }
#endif
}
