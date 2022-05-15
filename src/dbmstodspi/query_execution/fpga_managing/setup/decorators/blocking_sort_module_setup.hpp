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
#include "table_data.hpp"


namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class for noting which module has to fully sort the input table.
 */
class BlockingSortModuleSetup
    : public virtual AccelerationModuleSetupInterface {
 private:
  static void SortDataTableWhileMinimizingMinorRuns(
      const std::vector<int>& old_sorted_sequence,
      std::vector<int>& new_sorted_sequence, int record_count,
      int module_capacity);
  static void SortDataTableWhileMinimizingMajorRuns(
      const std::vector<int>& old_sorted_sequence,
      std::vector<int>& new_sorted_sequence, int record_count,
      int module_capacity);

 public:
  auto GetWorstCaseProcessedTables(
      const std::vector<int>& min_capacity,
      const std::vector<std::string>& input_tables,
      const std::map<std::string, TableMetadata>& data_tables)
      -> std::map<std::string, TableMetadata> override;
  auto UpdateDataTable(const std::vector<int>& module_capacity,
                       const std::vector<std::string>& input_table_names,
                       std::map<std::string, TableMetadata>& resulting_tables)
      -> bool override;
};
}  // namespace orkhestrafs::dbmstodspi
