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

#include "aggregation_sum_setup.hpp"

#include "aggregation_sum.hpp"
#include "aggregation_sum_interface.hpp"
#include "logger.hpp"

using orkhestrafs::dbmstodspi::AggregationSumSetup;

using orkhestrafs::dbmstodspi::AggregationSum;
using orkhestrafs::dbmstodspi::AggregationSumInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void AggregationSumSetup::SetupModule(
    AccelerationModule& acceleration_module,
    const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring sum on pos " +
          std::to_string(module_parameters.operation_module_location));
  AggregationSumSetup::SetupAggregationSum(
      dynamic_cast<AggregationSumInterface&>(acceleration_module),
      module_parameters.input_streams[0].stream_id,
      module_parameters.operation_parameters);
}

auto AggregationSumSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                       int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<AggregationSum>(memory_manager, module_postion);
}

void AggregationSumSetup::SetupAggregationSum(
    AggregationSumInterface& aggregation_module, int stream_id,
    const std::vector<std::vector<int>>& operation_parameters) {
  aggregation_module.ResetSumRegisters();
  aggregation_module.DefineInput(stream_id, operation_parameters.at(0).at(0));
  aggregation_module.StartPrefetching(false, false, false);
}
