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
#include <vector>

#include "operation_types.hpp"
#include "pr_module_data.hpp"

using orkhestrafs::core_interfaces::hw_library::OperationPRModules;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to preprocess the graph to improve performance
 */
class PreSchedulingProcessor {
 public:
  /**
   * @brief Method to get the smallest capacity values from the HW library
   * @param hw_library Map of PR modules available for each operation
   * @return Capacity values for each operation
   */
  static auto GetMinimumCapacityValuesFromHWLibrary(
      const std::map<QueryOperationType, OperationPRModules>& hw_library)
      -> std::map<QueryOperationType, std::vector<int>>;
};

}  // namespace orkhestrafs::dbmstodspi