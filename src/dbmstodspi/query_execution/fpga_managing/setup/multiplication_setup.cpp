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

#include "multiplication_setup.hpp"

#include "logger.hpp"
#include "multiplication.hpp"
#include "multiplication_interface.hpp"

using orkhestrafs::dbmstodspi::MultiplicationSetup;

using orkhestrafs::dbmstodspi::Multiplication;
using orkhestrafs::dbmstodspi::MultiplicationInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void MultiplicationSetup::SetupModule(
    AccelerationModule& acceleration_module,
    const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring multiplication on pos " +
          std::to_string(module_parameters.operation_module_location));
  MultiplicationSetup::SetupMultiplicationModule(
      dynamic_cast<MultiplicationInterface&>(acceleration_module),
      {module_parameters.input_streams[0].stream_id},
      module_parameters.operation_parameters);
}

auto MultiplicationSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                       int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<Multiplication>(memory_manager, module_postion);
}

void MultiplicationSetup::SetupMultiplicationModule(
    MultiplicationInterface& multiplication_module,
    const std::vector<int>& active_stream_ids,
    const std::vector<std::vector<int>>& operation_parameters) {
  std::bitset<16> active_streams;
  for (const auto& active_stream_id : active_stream_ids) {
    active_streams.set(active_stream_id, true);
  }
  multiplication_module.DefineActiveStreams(active_streams);

  for (const auto& multiplication_selections : operation_parameters) {
    std::bitset<8> selected_positions;
    for (int position = 0; position < 8; position++) {
      selected_positions.set(position,
                             multiplication_selections.at(8 - position) != 0);
    }
    multiplication_module.ChooseMultiplicationResults(
        multiplication_selections.at(0), selected_positions);
  }
}