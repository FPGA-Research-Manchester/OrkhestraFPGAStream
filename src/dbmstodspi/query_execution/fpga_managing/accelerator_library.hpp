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

#pragma once
#include <map>

#include "acceleration_module_setup_interface.hpp"
#include "accelerator_library_interface.hpp"

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Library to hold modules and their drivers.
 */
class AcceleratorLibrary : public AcceleratorLibraryInterface {
 public:
  ~AcceleratorLibrary() override = default;
  AcceleratorLibrary(MemoryManagerInterface* memory_manager_,
                     std::unique_ptr<DMASetupInterface> dma_setup,
                     std::map<QueryOperationType,
                              std::unique_ptr<AccelerationModuleSetupInterface>>
                         module_driver_library)
      : memory_manager_{memory_manager_},
        dma_setup_{std::move(dma_setup)},
        module_driver_library_{std::move(module_driver_library)} {};
  /**
   * @brief Setup a given operation with the given parameters.
   * @param node_parameters Parameters struct containing stream metadata and
   * resource elasticity information.
   */
  void SetupOperation(const AcceleratedQueryNode& node_parameters) override;
  /**
   * @brief Method to generate a DMA module for the FPGAManager to setup
   * execution.
   * @return DMA module class to access initialisation registers.
   */
  auto GetDMAModule() -> std::unique_ptr<DMAInterface> override;
  /**
   * @brief Check if the operation has readback modules and return them.
   * @return ReadBackModule to read if results have to be read from a register.
   */
  auto ExportLastModulesIfReadback()
      -> std::vector<std::unique_ptr<ReadBackModule>> override;
  /**
   * @brief Get the DMA module setup implementation
   * @return DMA driver setup class
   */
  auto GetDMAModuleSetup() -> DMASetupInterface& override;

  /**
   * @brief Get a pointer to an ILA module for debugging
   * @return Can return a nullpointer
   */
  auto GetILAModule() -> std::unique_ptr<ILA> override;

  /**
   * @brief Check if a stream is supposed to be a multi-channel stream
   * @param is_input Is the stream an input stream.
   * @param stream_index The index of the stream.
   * @param operation_type The operation performed on the stream
   * @return Is the stream multi-channel
   */
  auto IsMultiChannelStream(bool is_input, int stream_index,
                            QueryOperationType operation_type) -> bool override;

  /**
   * @brief Get multi channel parameters
   * @param is_input Is the stream an input stream.
   * @param stream_index The index of the stream.
   * @param operation_type The operation performed on the stream
   * @param operation_parameters Parameters where multi channel data can be
   * extracted from
   * @return How many channels with how many records there are for this stream.
   */
  auto GetMultiChannelParams(
      bool is_input, int stream_index, QueryOperationType operation_type,
      const std::vector<std::vector<int>>& operation_parameters)
      -> std::pair<int, int> override;

  /**
   * @brief Method to get capacity requirement values from the driver based on
   * operation parameters.
   * @param operation_type Operation type to find the correct driver.
   * @param operation_parameters Parameters to get the capacity from.
   * @return Vector holding the functional capacity requirement values.
   */
  auto GetNodeCapacity(
      QueryOperationType operation_type,
      const std::vector<std::vector<int>>& operation_parameters)
      -> std::vector<int> override;

  /**
   * @brief Method to check if the module to execute this operation needs to get
   * its input from the DMA.
   * @param operation_type Operation type
   * @return Boolean flag saying if it needs to be constrained position wise.
   */
  auto IsNodeConstrainedToFirstInPipeline(QueryOperationType operation_type)
      -> bool override;

  /**
   * @brief Check if the operation sorts the input table.
   * @param operation_type Operation type enum
   * @return Boolean flag
   */
  auto IsOperationSorting(QueryOperationType operation_type) -> bool override;

  /**
   * @brief Get minimum capacity requirements for a node to fully finish
   * sorting. Throws an error if the operation can't sort
   * @param table_data
   * @return
   */
  auto GetMinSortingRequirements(QueryOperationType operation_type,
                                 const TableMetadata& table_data)
      -> std::vector<int> override;

  /**
   * @brief Method to get worst case tables where tables with new metadata
   * values get created for calculating worst case scheduling plan.
   * @param operation_type Operation type of the module
   * @param min_capacity Minimum possible functional capacity the module library
   * provides for this operation.
   * @param input_tables Input tables to the module.
   * @param data_tables Metadata of all tables.
   * @return Map of metadata of returned tables.
   */
  auto GetWorstCaseProcessedTables(
      QueryOperationType operation_type, const std::vector<int>& min_capacity,
      const std::vector<std::string>& input_tables,
      const std::map<std::string, TableMetadata>& data_tables)
      -> std::map<std::string, TableMetadata> override;

  /**
   * Method to update tables for scheduling
   * @param operation_type Operation to update the tables.
   * @param module_capacity Capacity of the chosen module
   * @param input_table_names Input table names
   * @param data_tables All table data
   * @param resulting_tables Updated map
   * @return Boolean showing if the update was substantial enough to move on.
   */
  auto UpdateDataTable(
      QueryOperationType operation_type,
      const std::vector<int>& module_capacity,
      const std::vector<std::string>& input_table_names,
      const std::map<std::string, TableMetadata>& data_tables,
      std::map<std::string, TableMetadata>& resulting_tables) -> bool override;

  /**
   * Is the input table supposed to be sorted?
   * @param operation_type Operation enum
   * @return Boolean flag showing the requirement.
   */
  auto IsInputSupposedToBeSorted(QueryOperationType operation_type)
      -> bool override;

  /**
   * Method to select the output tables given input tables.
   * @param operation Operation enum
   * @param table_names Input table names
   * @param tables All table data
   * @return Vector of output table names.
   */
  auto GetResultingTables(
      QueryOperationType operation, const std::vector<std::string>& table_names,
      const std::map<std::string, TableMetadata>& tables)
      -> std::vector<std::string> override;

  /**
   * Method to check if the operation has a potential to reduce data size.
   * @param operation Operation enum
   * @return Boolean flag.
   */
  auto IsOperationReducingData(QueryOperationType operation)
      -> bool override;

  /**
   * Method to check if the operation is data sensive and requires precise input data.
   * @param operation Operation enum
   * @return Boolean flag.
   */
  auto IsOperationDataSensitive(QueryOperationType operation) -> bool override;

  /**
   * Method to get operation params for a passthrough module.
   * @param operation Operation enum
   * @param module_position Position of the passthrough module in the PR region.
   * @return Node with which a passthrough module can be initialised.
   */
  auto GetEmptyModuleNode(QueryOperationType operation, int module_position)
      -> AcceleratedQueryNode override;

 private:
  auto GetDriver(QueryOperationType operation_type)
      -> AccelerationModuleSetupInterface*;

  std::unique_ptr<DMASetupInterface> dma_setup_;
  std::map<QueryOperationType,
           std::unique_ptr<AccelerationModuleSetupInterface>>
      module_driver_library_;
  MemoryManagerInterface* memory_manager_;
  std::vector<std::unique_ptr<AccelerationModule>> recent_setup_modules_;
};

}  // namespace orkhestrafs::dbmstodspi