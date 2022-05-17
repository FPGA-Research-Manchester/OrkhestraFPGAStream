/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "fpga_manager.hpp"

#include <iostream>
#include <stdexcept>

#ifdef FPGA_AVAILABLE
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
#include "logger.hpp"
#include "merge_sort.hpp"
#include "merge_sort_setup.hpp"
#include "multiplication.hpp"
#include "multiplication_setup.hpp"
#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::dbmstodspi::AcceleratedQueryNode;
using orkhestrafs::dbmstodspi::FPGAManager;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void FPGAManager::SetupQueryAcceleration(
    const std::vector<AcceleratedQueryNode>& query_nodes) {
  // TODO(Kaspar): Doesn't reset configuration. Unused modules should be
  // configured to be unused.
  dma_engine_->GlobalReset();
  read_back_modules_.clear();
  read_back_parameters_.clear();

  // if (ila_module_) {
  //  ila_module_->StartAxiILA();
  //}

  std::vector<StreamDataParameters> input_streams;
  std::vector<StreamDataParameters> output_streams;
  for (const auto& query_node : query_nodes) {
    bool is_multichannel_stream =
        query_node.operation_type == QueryOperationType::kMergeSort;
    FindIOStreams(query_node.input_streams, input_streams,
                  query_node.operation_parameters, is_multichannel_stream,
                  input_streams_active_status_);
    FindIOStreams(query_node.output_streams, output_streams,
                  query_node.operation_parameters, false,
                  output_streams_active_status_);
    /*input_streams_active_status_[0] = false;
    input_streams_active_status_[1] = true;
    output_streams_active_status_[0] = false;
    output_streams_active_status_[2] = true;
    output_streams_active_status_[4] = true;
    output_streams.push_back(output_streams[0]);*/
  }

  // MISSING PIECE OF LOGIC HERE...
  // TODO(Kaspar): Need to check for stream specification validity.

  if (input_streams.empty() || output_streams.empty()) {
    throw std::runtime_error("Input or output streams missing!");
  }
  dma_setup_.SetupDMAModule(*dma_engine_, input_streams, output_streams);

  dma_engine_->StartController(true, input_streams_active_status_);

  if (ila_module_) {
    ila_module_->StartILAs();
  }

  for (const auto& query_node : query_nodes) {
    module_library_->SetupOperation(query_node);
    auto readback_modules = module_library_->ExportLastModulesIfReadback();
    if (!readback_modules.empty()) {
      for (auto& readback_module : readback_modules) {
        read_back_modules_.push_back(std::move(readback_module));
        // Don't know how this will work out with combined readback modules as
        // we don't have any at the moment.
        read_back_parameters_.push_back(query_node.operation_parameters.at(1));
      }
    }
  }
}

void FPGAManager::FindIOStreams(
    const std::vector<StreamDataParameters>& all_streams,
    std::vector<StreamDataParameters>& found_streams,
    const std::vector<std::vector<int>>& /*operation_parameters*/,
    const bool /*is_multichannel_stream*/,
    std::bitset<query_acceleration_constants::kMaxIOStreamCount>&
        stream_status_array) {
  for (const auto& current_stream : all_streams) {
    // Stream ID 15 reserved for passthrough
    if (current_stream.physical_address && current_stream.stream_id != 15) {
      found_streams.push_back(current_stream);
      stream_status_array[current_stream.stream_id] = true;
      // To configure passthrough streams
      /*auto new_stream = current_stream;
      new_stream.stream_record_count = 0;
      new_stream.stream_id = 1;
      found_streams.push_back(new_stream);
      stream_status_array[new_stream.stream_id] = true;*/
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

#ifdef FPGA_AVAILABLE
  ReadResultsFromRegisters();
#endif

  Log(LogLevel::kInfo,
      "Execution time = " +
          std::to_string(
              std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
                  .count()) +
          "[microseconds]");

  //PrintDebuggingData();
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
  dma_engine_->StartController(false, output_streams_active_status_);

  /*auto test = dma_engine_->IsControllerFinished(true);
  auto test1 = dma_engine_->IsControllerFinished(true);
  auto test2 = dma_engine_->IsControllerFinished(false);
  auto test3 = dma_engine_->IsControllerFinished(false);*/

#ifdef FPGA_AVAILABLE
  while (!(dma_engine_->IsControllerFinished(true) &&
           dma_engine_->IsControllerFinished(false))) {
    // sleep(3);
    // std::cout << "Processing..." << std::endl;
    // std::cout << "Input:"
    //          << dma_engine_.IsInputControllerFinished()
    //          << std::endl;
    // std::cout << "Output:"
    //          << dma_engine_.IsOutputControllerFinished()
    //          << std::endl;
  }
#endif
}

void FPGAManager::ReadResultsFromRegisters() {
  if (!read_back_modules_.empty()) {
    // Assuming there are equal number of read back modules and parameters
    for (int module_index = 0; module_index < read_back_modules_.size();
         module_index++) {
      for (auto const& position : read_back_parameters_.at(module_index)) {
        std::cout << "SUM: " << std::fixed << std::setprecision(2)
                  << ReadModuleResultRegisters(
                         std::move(read_back_modules_.at(module_index)),
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
    input_streams_active_status_[stream_id] = false;
  }
  std::array<int, query_acceleration_constants::kMaxIOStreamCount>
      result_sizes{};
  for (auto stream_id : active_output_stream_ids) {
    output_streams_active_status_[stream_id] = false;
    /*if (stream_id == 2) {
      result_sizes[stream_id - 2] =
          dma_engine_->GetControllerStreamSize(false, stream_id);
      int yo = 0;
    } else {
      auto thing = dma_engine_->GetControllerStreamSize(false, stream_id);
      int yo = 0;
    }*/
    result_sizes[stream_id] =
        dma_engine_->GetControllerStreamSize(false, stream_id);
  }
  return result_sizes;
}

void FPGAManager::PrintDebuggingData() {
#ifdef FPGA_AVAILABLE
  auto log_level = LogLevel::kDebug;
  Log(log_level, "Runtime: " + std::to_string(dma_engine_->GetRuntime()));
  Log(log_level, "ValidReadCount: " +
                     std::to_string(dma_engine_->GetValidReadCyclesCount()));
  Log(log_level, "ValidWriteCount: " +
                     std::to_string(dma_engine_->GetValidWriteCyclesCount()));

  Log(log_level, "InputActiveDataCycles: " +
                     std::to_string(dma_engine_->GetInputActiveDataCycles()));
  Log(log_level,
      "InputActiveDataLastCycles: " +
          std::to_string(dma_engine_->GetInputActiveDataLastCycles()));
  Log(log_level,
      "InputActiveControlCycles: " +
          std::to_string(dma_engine_->GetInputActiveControlCycles()));
  Log(log_level,
      "InputActiveControlLastCycles: " +
          std::to_string(dma_engine_->GetInputActiveControlLastCycles()));
  Log(log_level,
      "InputActiveEndOfStreamCycles: " +
          std::to_string(dma_engine_->GetInputActiveEndOfStreamCycles()));
  Log(log_level, "OutputActiveDataCycles : " +
                     std::to_string(dma_engine_->GetOutputActiveDataCycles()));
  Log(log_level,
      "OutputActiveDataLastCycles: " +
          std::to_string(dma_engine_->GetOutputActiveDataLastCycles()));
  Log(log_level,
      "OutputActiveControlCycles: " +
          std::to_string(dma_engine_->GetOutputActiveControlCycles()));
  Log(log_level,
      "OutputActiveControlLastCycles: " +
          std::to_string(dma_engine_->GetOutputActiveControlLastCycles()));
  Log(log_level,
      "OutputActiveEndOfStreamCycles : " +
          std::to_string(dma_engine_->GetOutputActiveEndOfStreamCycles()));
  Log(log_level,
      "InputActiveInstructionCycles : " +
          std::to_string(dma_engine_->GetInputActiveInstructionCycles()));
  Log(log_level,
      "OutputActiveInstructionCycles : " +
          std::to_string(dma_engine_->GetOutputActiveInstructionCycles()));
  if (ila_module_) {
    std::cout << "======================================================ILA 0 "
                 "DATA ======================================================="
              << std::endl;
    ila_module_->PrintILAData(0, 2048);
    std::cout << "======================================================ILA 1 "
                 "DATA ======================================================="
              << std::endl;
    ila_module_->PrintILAData(1, 2048);
    std::cout << "======================================================ILA 2 "
                 "DATA ======================================================="
              << std::endl;
    ila_module_->PrintDMAILAData(2048);
  }
#endif
}

auto FPGAManager::ReadModuleResultRegisters(
    std::unique_ptr<ReadBackModule> read_back_module, int position) -> double {
  // Reversed reading because the data is reversed.
  uint32_t high_bits = read_back_module->ReadResult(
      ((query_acceleration_constants::kDatapathWidth - 1) - position * 2));
  uint32_t low_bits = read_back_module->ReadResult((
      (query_acceleration_constants::kDatapathWidth - 1) - (position * 2 + 1)));

  return static_cast<long long>((static_cast<uint64_t>(high_bits) << 32) +
                                low_bits) /
         100.0;
}
