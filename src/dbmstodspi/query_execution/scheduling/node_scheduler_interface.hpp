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
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface for schedulers
 */
class NodeSchedulerInterface {
 public:
  virtual ~NodeSchedulerInterface() = default;
  /**
   * @brief Find groups of accelerated query nodes which can be run in the same
   * FPGA run.
   * @param starting_nodes Input vector of leaf nodes from which the parsing can
   * begin.
   * @param supported_accelerator_bitstreams Map of hardware module combinations
   * which have a corresponding bitstream.
   * @param existing_modules_library Map of hardware modules and the available
   * variations with different computational capacity values.
   * @return Queue of groups of accelerated query
   * nodes to be accelerated next.
   */
  virtual auto FindAcceleratedQueryNodeSets(
      std::vector<std::shared_ptr<QueryNode>> starting_nodes,
      const std::map<ConfigurableModulesVector, std::string>&
          supported_accelerator_bitstreams,
      const std::map<QueryOperationType, std::vector<std::vector<int>>>&
          existing_modules_library,
      std::map<std::string,
               std::map<int, std::vector<std::pair<std::string, int>>>>&
          linked_nodes)
      -> std::queue<std::pair<ConfigurableModulesVector,
                              std::vector<std::shared_ptr<QueryNode>>>> = 0;
};
}  // namespace orkhestrafs::dbmstodspi