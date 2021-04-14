#include "fpga_manager.hpp"

#include <iostream>
#include <stdexcept>

#ifdef _FPGA_AVAILABLE
#include <unistd.h>
#endif

#include <chrono>

#include "addition.hpp"
#include "addition_setup.hpp"
#include "dma_setup.hpp"
#include "filter.hpp"
#include "filter_setup.hpp"
#include "ila.hpp"
#include "join.hpp"
#include "join_setup.hpp"
#include "linear_sort.hpp"
#include "linear_sort_setup.hpp"
#include "merge_sort.hpp"
#include "merge_sort_setup.hpp"
#include "multiplication.hpp"
#include "multiplication_setup.hpp"
#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"

using namespace dbmstodspi::fpga_managing;

// Eventually the idea would be to add operation type to StreamDataParameters.
// Then the modules and streams can be set up accordingly.
void FPGAManager::SetupQueryAcceleration(
    const std::vector<operation_types::QueryOperation>& available_modules,
    const std::vector<AcceleratedQueryNode>& query_nodes) {
  dma_engine_.GlobalReset();

  // ila_module_ = std::make_optional (ILA(memory_manager_));
  /*if (ila_module_) {
    ila_module_.value().startAxiILA();
  }*/

  std::vector<StreamDataParameters> input_streams;
  std::vector<StreamDataParameters> output_streams;
  for (const auto& query_node : query_nodes) {
    bool is_multichannel_stream = query_node.operation_type ==
                                  operation_types::QueryOperation::kMergeSort;
    FindIOStreams(query_node.input_streams, input_streams,
                  query_node.operation_parameters, is_multichannel_stream,
                  input_streams_active_status_);
    FindIOStreams(query_node.output_streams, output_streams,
                  query_node.operation_parameters, false,
                  output_streams_active_status_);
  }

  // MISSING PIECE OF LOGIC HERE...
  // Need to check for stream specification validity and intermediate stream
  // specifications have to be merged with the IO stream specs.

  if (input_streams.empty() || output_streams.empty()) {
    throw std::runtime_error("Input or output streams missing!");
  }
  DMASetup::SetupDMAModule(dma_engine_, input_streams, true);
  DMASetup::SetupDMAModule(dma_engine_, output_streams, false);

  FPGAManager::dma_engine_.StartInputController(
      FPGAManager::input_streams_active_status_);

  if (ila_module_) {
    ila_module_.value().StartILAs();
  }

  for (int node_index = 0; node_index < available_modules.size();
       node_index++) {
    // Find out which module is associated with which query node. Current method
    // doesn't work with multiple identical modules!
    const AcceleratedQueryNode* current_query_node = nullptr;
    for (const auto& query_node : query_nodes) {
      if (query_node.operation_type == available_modules[node_index]) {
        current_query_node = &query_node;
      }
    }
    int module_location = node_index + 1;
    auto query_node = *current_query_node;

    // Assumptions are taken that the input and output streams are defined
    // correctly and that the correct amount of streams have been given for each
    // operation.
    switch (query_node.operation_type) {
      case operation_types::QueryOperation::kFilter: {
        modules::Filter filter_module(memory_manager_, module_location);
        FilterSetup::SetupFilterModule(filter_module,
                                       query_node.input_streams[0].stream_id,
                                       query_node.output_streams[0].stream_id,
                                       query_node.operation_parameters);
        break;
      }
      case operation_types::QueryOperation::kJoin: {
        modules::Join join_module(memory_manager_, module_location);
        JoinSetup::SetupJoinModule(
            join_module, query_node.input_streams[0].stream_id,
            GetStreamRecordSize(query_node.input_streams[0]),
            query_node.input_streams[1].stream_id,
            GetStreamRecordSize(query_node.input_streams[1]),
            query_node.output_streams[0].stream_id,
            query_node.output_streams[0].input_chunks_per_record,
            query_node.operation_parameters.at(0).at(0));
        break;
      }
      case operation_types::QueryOperation::kMergeSort: {
        modules::MergeSort merge_sort_module(memory_manager_, module_location);
        MergeSortSetup::SetupMergeSortModule(
            merge_sort_module, query_node.input_streams[0].stream_id,
            GetStreamRecordSize(query_node.input_streams[0]), 0, true);
        break;
      }
      case operation_types::QueryOperation::kLinearSort: {
        modules::LinearSort linear_sort_module(memory_manager_,
                                               module_location);
        LinearSortSetup::SetupLinearSortModule(
            linear_sort_module, query_node.input_streams[0].stream_id,
            GetStreamRecordSize(query_node.input_streams[0]));
        break;
      }
      case operation_types::QueryOperation::kAddition: {
        modules::Addition addition_module(memory_manager_, module_location);
        AdditionSetup::SetupAdditionModule(
            addition_module, query_node.input_streams[0].stream_id);
        break;
      }
      case operation_types::QueryOperation::kMultiplication: {
        modules::Multiplication multiplication_module(memory_manager_, module_location);
        MultiplicationSetup::SetupMultiplicationModule(
            multiplication_module, query_node.input_streams[0].stream_id);
        break;
      }
    }
  }
}

void FPGAManager::FindIOStreams(
    const std::vector<StreamDataParameters>& all_streams,
    std::vector<StreamDataParameters>& found_streams,
    const std::vector<std::vector<int>>& operation_parameters,
    const bool is_multichannel_stream, bool stream_status_array[]) {
  for (const auto& current_stream : all_streams) {
    if (current_stream.physical_address) {
      if (is_multichannel_stream) {
        // Assumption is that there is only one multichannel stream in this node
        // which gets the multi channel parameters
        found_streams.push_back(
            {current_stream.stream_id, current_stream.stream_record_size,
             current_stream.stream_record_count,
             current_stream.physical_address,
             current_stream.stream_specification,
             current_stream.input_chunks_per_record, operation_parameters[0][0],
             operation_parameters[0][1]});
      } else {
        found_streams.push_back(current_stream);
      }
      stream_status_array[current_stream.stream_id] = true;
    }
  }
}

auto FPGAManager::RunQueryAcceleration() -> std::vector<int> {
  std::vector<int> active_input_stream_ids;
  std::vector<int> active_output_stream_ids;

  FindActiveStreams(active_input_stream_ids, active_output_stream_ids);

  if (active_input_stream_ids.empty() || active_output_stream_ids.empty()) {
    throw std::runtime_error("FPGA does not have active streams!");
  }

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  WaitForStreamsToFinish();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Execution time = "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << "[µs]" << std::endl;

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
    // sleep(3);
    // std::cout << "Processing..." << std::endl;
    // std::cout << "Input:"
    //          << FPGAManager::dma_engine_.IsInputControllerFinished()
    //          << std::endl;
    // std::cout << "Output:"
    //          << FPGAManager::dma_engine_.IsOutputControllerFinished()
    //          << std::endl;
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

auto FPGAManager::GetStreamRecordSize(
    const StreamDataParameters& stream_parameters) -> int {
  if (stream_parameters.stream_specification.empty()) {
    return stream_parameters.stream_record_size;
  }
  return stream_parameters.stream_specification.size();
}
