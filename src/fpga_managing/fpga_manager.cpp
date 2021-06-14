#include "fpga_manager.hpp"

#include <iostream>
#include <stdexcept>

#ifdef _FPGA_AVAILABLE
#include <unistd.h>
#endif

#include <chrono>
#include <iomanip>

#include "addition.hpp"
#include "addition_setup.hpp"
#include "aggregation_sum.hpp"
#include "aggregation_sum_setup.hpp"
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

void FPGAManager::SetupQueryAcceleration(
    const std::vector<AcceleratedQueryNode>& query_nodes) {
  // Doesn't reset configuration. Unused modules should be configured to be
  // unused.
  dma_engine_.GlobalReset();
  read_back_modules_.clear();
  read_back_parameters_.clear();

  // ila_module_ = std::make_optional (ILA(memory_manager_));
  /*if (ila_module_) {
    ila_module_.value().startAxiILA();
  }*/

  std::vector<StreamDataParameters> input_streams;
  std::vector<StreamDataParameters> output_streams;
  for (const auto& query_node : query_nodes) {
    bool is_multichannel_stream =
        query_node.operation_type ==
        operation_types::QueryOperationType::kMergeSort;
    FindIOStreams(query_node.input_streams, input_streams,
                  query_node.operation_parameters, is_multichannel_stream,
                  input_streams_active_status_);
    FindIOStreams(query_node.output_streams, output_streams,
                  query_node.operation_parameters, false,
                  output_streams_active_status_);
  }

  // MISSING PIECE OF LOGIC HERE...
  // TODO(Kaspar): Need to check for stream specification validity and
  // intermediate stream specifications have to be merged with the IO stream
  // specs.

  if (input_streams.empty() || output_streams.empty()) {
    throw std::runtime_error("Input or output streams missing!");
  }
  DMASetup::SetupDMAModule(dma_engine_, input_streams, true);
  DMASetup::SetupDMAModule(dma_engine_, output_streams, false);

  FPGAManager::dma_engine_.StartController(
      true, FPGAManager::input_streams_active_status_);

  if (ila_module_) {
    ila_module_.value().StartILAs();
  }

  for (const auto& query_node : query_nodes) {
    // TODO(Kaspar): For each of the operations different properties have to be
    // written down to add classifications. The operation_parameters can be
    // generalised based on classifiactions

    // Assumptions are taken that the input and output streams are defined
    // correctly and that the correct amount of streams have been given for each
    // operation.

    // Make it possible to configure a module to be unused.
    switch (query_node.operation_type) {
      case operation_types::QueryOperationType::kFilter: {
        modules::Filter filter_module(memory_manager_,
                                      query_node.operation_module_location);
        FilterSetup::SetupFilterModule(filter_module,
                                       query_node.input_streams[0].stream_id,
                                       query_node.output_streams[0].stream_id,
                                       query_node.operation_parameters);
        break;
      }
      case operation_types::QueryOperationType::kJoin: {
        modules::Join join_module(memory_manager_,
                                  query_node.operation_module_location);
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
      case operation_types::QueryOperationType::kMergeSort: {
        modules::MergeSort merge_sort_module(
            memory_manager_, query_node.operation_module_location);
        MergeSortSetup::SetupMergeSortModule(
            merge_sort_module, query_node.input_streams[0].stream_id,
            GetStreamRecordSize(query_node.input_streams[0]), 0, true);
        break;
      }
      case operation_types::QueryOperationType::kLinearSort: {
        modules::LinearSort linear_sort_module(
            memory_manager_, query_node.operation_module_location);
        LinearSortSetup::SetupLinearSortModule(
            linear_sort_module, query_node.input_streams[0].stream_id,
            GetStreamRecordSize(query_node.input_streams[0]));
        break;
      }
      case operation_types::QueryOperationType::kAddition: {
        modules::Addition addition_module(memory_manager_,
                                          query_node.operation_module_location);
        AdditionSetup::SetupAdditionModule(
            addition_module, query_node.input_streams[0].stream_id,
            query_node.operation_parameters);
        break;
      }
      case operation_types::QueryOperationType::kMultiplication: {
        modules::Multiplication multiplication_module(
            memory_manager_, query_node.operation_module_location);
        MultiplicationSetup::SetupMultiplicationModule(
            multiplication_module, {query_node.input_streams[0].stream_id},
            query_node.operation_parameters);
        break;
      }
      case operation_types::QueryOperationType::kAggregationSum: {
        auto aggregation_module = std::make_unique<modules::AggregationSum>(
            memory_manager_, query_node.operation_module_location);
        AggregationSumSetup::SetupAggregationSum(
            *aggregation_module, query_node.input_streams[0].stream_id,
            query_node.operation_parameters);
        read_back_modules_.push_back(std::move(aggregation_module));
        read_back_parameters_.push_back(query_node.operation_parameters.at(1));
        break;
      }
    }
  }
}

void FPGAManager::FindIOStreams(
    const std::vector<StreamDataParameters>& all_streams,
    std::vector<StreamDataParameters>& found_streams,
    const std::vector<std::vector<int>>& operation_parameters,
    const bool is_multichannel_stream,
    std::bitset<query_acceleration_constants::kMaxIOStreamCount>&
        stream_status_array) {
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

auto FPGAManager::RunQueryAcceleration()
    -> std::array<int, query_acceleration_constants::kMaxIOStreamCount> {
  std::vector<int> active_input_stream_ids;
  std::vector<int> active_output_stream_ids;

  // This can be expanded on in the future with multi threaded processing where
  // some streams are checked while others are being setup and fired.
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
            << "[microseconds]" << std::endl;

  // PrintDebuggingData();
  return GetResultingStreamSizes(active_input_stream_ids,
                                 active_output_stream_ids);
}

void FPGAManager::FindActiveStreams(
    std::vector<int>& active_input_stream_ids,
    std::vector<int>& active_output_stream_ids) {
  for (int stream_id = 0;
       stream_id < query_acceleration_constants::kMaxIOStreamCount;
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
  FPGAManager::dma_engine_.StartController(
      false, FPGAManager::output_streams_active_status_);

#ifdef _FPGA_AVAILABLE
  while (!(FPGAManager::dma_engine_.IsControllerFinished(true) &&
           FPGAManager::dma_engine_.IsControllerFinished(false))) {
    // sleep(3);
    // std::cout << "Processing..." << std::endl;
    // std::cout << "Input:"
    //          << FPGAManager::dma_engine_.IsInputControllerFinished()
    //          << std::endl;
    // std::cout << "Output:"
    //          << FPGAManager::dma_engine_.IsOutputControllerFinished()
    //          << std::endl;
  }
  ReadResultsFromRegisters();
#endif
}

void FPGAManager::ReadResultsFromRegisters() {
  if (!FPGAManager::read_back_modules_.empty()) {
    // Assuming there are equal number of read back modules and parameters
    for (int module_index = 0;
         module_index < FPGAManager::read_back_modules_.size();
         module_index++) {
      for (auto const& position :
           FPGAManager::read_back_parameters_.at(module_index)) {
        std::cout << "SUM: " << std::fixed << std::setprecision(2)
                  << ReadModuleResultRegisters(
                         std::move(
                             FPGAManager::read_back_modules_.at(module_index)),
                         position)
                  << std::endl;
      }
    }
  }
}

auto FPGAManager::GetResultingStreamSizes(
    const std::vector<int>& active_input_stream_ids,
    const std::vector<int>& active_output_stream_ids)
    -> std::array<int, query_acceleration_constants::kMaxIOStreamCount> {
  for (auto stream_id : active_input_stream_ids) {
    FPGAManager::input_streams_active_status_[stream_id] = false;
  }
  std::array<int, query_acceleration_constants::kMaxIOStreamCount>
      result_sizes{};
  for (auto stream_id : active_output_stream_ids) {
    FPGAManager::output_streams_active_status_[stream_id] = false;
    result_sizes[stream_id] =
        dma_engine_.GetControllerStreamSize(false, stream_id);
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

auto dbmstodspi::fpga_managing::FPGAManager::ReadModuleResultRegisters(
    std::unique_ptr<modules::ReadBackModuleInterface> read_back_module,
    int position) -> double {
  // Reversed reading because the data is reversed.
  uint32_t high_bits = read_back_module->ReadResult(
      ((query_acceleration_constants::kDatapathWidth - 1) - position * 2));
  uint32_t low_bits = read_back_module->ReadResult((
      (query_acceleration_constants::kDatapathWidth - 1) - (position * 2 + 1)));

  return static_cast<long long>((static_cast<uint64_t>(high_bits) << 32) +
                                low_bits) /
         100.0;
}
