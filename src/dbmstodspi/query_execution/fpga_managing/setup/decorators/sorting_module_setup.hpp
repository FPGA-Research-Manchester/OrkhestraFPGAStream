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

using orkhestrafs::core_interfaces::table_data::TableMetadata;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class for noting which module has to sort the input table.
 */
class SortingModuleSetup : public virtual AccelerationModuleSetupInterface {
 public:
  auto IsSortingInputTable() -> bool override;
  virtual auto GetMinSortingRequirementsForTable(
      const TableMetadata& table_data) -> std::vector<int> = 0;
};
}  // namespace orkhestrafs::dbmstodspi
