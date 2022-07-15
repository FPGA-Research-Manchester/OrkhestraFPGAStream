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
#include "acceleration_module_setup_interface.hpp"
#include "linear_sort_interface.hpp"
#include "sorting_module_setup.hpp"
#include "table_data.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Linear sort setup class which will calculate the configuration data to
 * setup the module
 */
class LinearSortSetup : public virtual AccelerationModuleSetupInterface,
                        public SortingModuleSetup {
 private:
  static void GetSortedSequenceWithCapacity(int bitstream_capacity,
                                            int record_count, std::vector<int>& sorted_sequence);

 public:
  void SetupModule(AccelerationModule& acceleration_module,
                   const AcceleratedQueryNode& module_parameters) override;
  auto CreateModule(MemoryManagerInterface* memory_manager, int module_postion)
      -> std::unique_ptr<AccelerationModule> override;
  auto GetMinSortingRequirementsForTable(const TableMetadata& table_data)
      -> std::vector<int> override;
  auto GetWorstCaseProcessedTables(
      const std::vector<int>& min_capacity,
      const std::vector<std::string>& input_tables,
      const std::map<std::string, TableMetadata>& data_tables,
      const std::vector<std::string>& output_table_names)
      -> std::map<std::string, TableMetadata> override;
  auto UpdateDataTable(const std::vector<int>& module_capacity,
                       const std::vector<std::string>& input_table_names,
                       std::map<std::string, TableMetadata>& resulting_tables)
      -> bool override;
  auto GetWorstCaseNodeCapacity(const std::vector<int>& min_capacity,
      const std::vector<std::string>& input_tables,
      const std::map<std::string, TableMetadata>& data_tables,
      QueryOperationType next_operation_type) -> std::vector<int> override;
  /**
   * @brief Setup linear sort module by giving the stream data to be sorted.
   * @param linear_sort_module Module instance to access the configuration
   * registers.
   * @param stream_id ID of the stream to be sorted.
   * @param record_size How many integers worth of data does a record have.
   */
  static void SetupLinearSortModule(LinearSortInterface& linear_sort_module,
                                    int stream_id, int record_size);
};

}  // namespace orkhestrafs::dbmstodspi