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

#include "linear_sort_setup.hpp"

#include "linear_sort.hpp"
#include "linear_sort_interface.hpp"
#include "logger.hpp"
#include "stream_parameter_calculator.hpp"

using orkhestrafs::dbmstodspi::LinearSortSetup;

using orkhestrafs::dbmstodspi::LinearSort;
using orkhestrafs::dbmstodspi::LinearSortInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void LinearSortSetup::SetupModule(
    AccelerationModule& acceleration_module,
    const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring linear sort on pos " +
          std::to_string(module_parameters.operation_module_location));
  LinearSortSetup::SetupLinearSortModule(
      dynamic_cast<LinearSortInterface&>(acceleration_module),
      module_parameters.input_streams[0].stream_id,
      GetStreamRecordSize(module_parameters.input_streams[0]));
}

auto LinearSortSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                   int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<LinearSort>(memory_manager, module_postion);
}

void LinearSortSetup::SetupLinearSortModule(
    LinearSortInterface& linear_sort_module, int stream_id, int record_size) {
  int chunks_per_record =
      StreamParameterCalculator::CalculateChunksPerRecord(record_size);

  linear_sort_module.SetStreamParams(stream_id, chunks_per_record);

  linear_sort_module.StartPrefetchingData();
}

auto LinearSortSetup::GetMinSortingRequirementsForTable(
    const TableMetadata& table_data) -> std::vector<int> {
  return {table_data.record_count};
}