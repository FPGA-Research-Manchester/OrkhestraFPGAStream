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

#include "accelerator_library.hpp"

#include <stdexcept>

#include "sorting_module_setup.hpp"

using orkhestrafs::dbmstodspi::AcceleratorLibrary;

void AcceleratorLibrary::SetupOperation(
    const AcceleratedQueryNode& node_parameters) {
  auto* driver = GetDriver(node_parameters.operation_type);
  recent_setup_modules_.clear();
  // TODO(Kaspar): Put into a for loop for composed modules.
  auto current_module = driver->CreateModule(
      memory_manager_, node_parameters.operation_module_location);
  driver->SetupModule(*current_module, node_parameters);
  recent_setup_modules_.push_back(std::move(current_module));
}

auto AcceleratorLibrary::GetDMAModule() -> std::unique_ptr<DMAInterface> {
  return dma_setup_->CreateDMAModule(memory_manager_);
}

auto AcceleratorLibrary::ExportLastModulesIfReadback()
    -> std::vector<std::unique_ptr<ReadBackModule>> {
  std::vector<std::unique_ptr<ReadBackModule>> resulting_vector;
  for (auto& recent_setup_module : recent_setup_modules_) {
    auto* cast_check = dynamic_cast<ReadBackModule*>(recent_setup_module.get());
    if (cast_check) {
      std::unique_ptr<ReadBackModule> my_new_pointer(
          dynamic_cast<ReadBackModule*>(
              std::move(recent_setup_module).release()));
      resulting_vector.push_back(std::move(my_new_pointer));
    }
  }
  recent_setup_modules_.clear();
  return resulting_vector;
}

auto AcceleratorLibrary::GetDMAModuleSetup() -> DMASetupInterface& {
  return *dma_setup_;
}

auto AcceleratorLibrary::GetILAModule() -> std::unique_ptr<ILA> {
  // return std::make_unique<ILA>(memory_manager_.get());
  return nullptr;
}
auto AcceleratorLibrary::IsMultiChannelStream(bool is_input, int stream_index,
                                              QueryOperationType operation_type)
    -> bool {
  auto* driver = GetDriver(operation_type);
  return driver->IsMultiChannelStream(is_input, stream_index);
}
auto AcceleratorLibrary::GetMultiChannelParams(
    bool is_input, int stream_index, QueryOperationType operation_type,
    const std::vector<std::vector<int>>& operation_parameters)
    -> std::pair<int, std::vector<int>> {
  auto* driver = GetDriver(operation_type);
  return driver->GetMultiChannelParams(is_input, stream_index,
                                       operation_parameters);
}

auto AcceleratorLibrary::GetDriver(QueryOperationType operation_type)
    -> AccelerationModuleSetupInterface* {
  auto search = module_driver_library_.find(operation_type);
  if (search != module_driver_library_.end()) {
    return search->second.get();
  }
  throw std::runtime_error("Operator driver not found!");
}

auto AcceleratorLibrary::GetNodeCapacity(
    QueryOperationType operation_type,
    const std::vector<std::vector<int>>& operation_parameters)
    -> std::vector<int> {
  auto* driver = GetDriver(operation_type);
  return driver->GetCapacityRequirement(operation_parameters);
}

auto AcceleratorLibrary::IsNodeConstrainedToFirstInPipeline(
    QueryOperationType operation_type) -> bool {
  auto* driver = GetDriver(operation_type);
  return driver->IsConstrainedToFirstInPipeline();
}

auto AcceleratorLibrary::IsOperationSorting(QueryOperationType operation_type)
    -> bool {
  auto* driver = GetDriver(operation_type);
  return driver->IsSortingInputTable();
}

auto AcceleratorLibrary::GetMinSortingRequirements(
    QueryOperationType operation_type, const TableMetadata& table_data)
    -> std::vector<int> {
  auto* driver = dynamic_cast<SortingModuleSetup*>(GetDriver(operation_type));
  if (driver) {
    return driver->GetMinSortingRequirementsForTable(table_data);
  }
  throw std::runtime_error("Non sorting operation given!");
}

auto AcceleratorLibrary::GetWorstCaseProcessedTables(
    QueryOperationType operation_type, const std::vector<int>& min_capacity,
    const std::vector<std::string>& input_tables,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::vector<std::string>& output_table_names)
    -> std::map<std::string, TableMetadata> {
  auto* driver = GetDriver(operation_type);
  return driver->GetWorstCaseProcessedTables(min_capacity, input_tables,
                                             data_tables, output_table_names);
}

auto AcceleratorLibrary::UpdateDataTable(
    QueryOperationType operation_type, const std::vector<int>& module_capacity,
    const std::vector<std::string>& input_table_names,
    std::map<std::string, TableMetadata>& resulting_tables) -> bool {
  auto* driver = GetDriver(operation_type);
  return driver->UpdateDataTable(module_capacity, input_table_names,
                                 resulting_tables);
}

auto AcceleratorLibrary::SetMissingFunctionalCapacity(
    const std::vector<int>& bitstream_capacity,
    std::vector<int>& missing_capacity, const std::vector<int>& node_capacity,
    bool is_composed,
    QueryOperationType operation_type) -> bool {
  auto* driver = GetDriver(operation_type);
  return driver->SetMissingFunctionalCapacity(bitstream_capacity,
                                              missing_capacity, node_capacity, is_composed);
}

auto AcceleratorLibrary::IsInputSupposedToBeSorted(
    QueryOperationType operation_type) -> bool {
  auto* driver = GetDriver(operation_type);
  return driver->InputHasToBeSorted();
}

auto AcceleratorLibrary::GetResultingTables(
    QueryOperationType operation, const std::vector<std::string>& table_names,
    const std::map<std::string, TableMetadata>& tables)
    -> std::vector<std::string> {
  auto* driver = GetDriver(operation);
  return driver->GetResultingTables(tables, table_names);
}

auto AcceleratorLibrary::IsOperationReducingData(QueryOperationType operation)
    -> bool {
  auto* driver = GetDriver(operation);
  return driver->IsReducingData();
}

auto AcceleratorLibrary::IsOperationDataSensitive(QueryOperationType operation)
    -> bool {
  auto* driver = GetDriver(operation);
  return driver->IsDataSensitive();
}

auto AcceleratorLibrary::GetEmptyModuleNode(QueryOperationType operation,
                                            int module_position)
    -> AcceleratedQueryNode {
  auto* driver = GetDriver(operation);
  AcceleratedQueryNode passthrough_module_node =
      driver->GetPassthroughInitParameters();
  passthrough_module_node.operation_type = operation;
  passthrough_module_node.operation_module_location = module_position;
  passthrough_module_node.composed_module_locations = {};
  return passthrough_module_node;
}

auto AcceleratorLibrary::GetWorstCaseNodeCapacity(
    QueryOperationType operation_type, const std::vector<int>& min_capacity,
    const std::vector<std::string>& input_tables,
    const std::map<std::string, TableMetadata>& data_tables,
    QueryOperationType next_operation_type) -> std::vector<int> {
  auto* driver = GetDriver(operation_type);
  return driver->GetWorstCaseNodeCapacity(min_capacity, input_tables,
                                          data_tables, next_operation_type);
}
