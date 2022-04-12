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

#include <string>

#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::core_interfaces {
/**
 * @brief Interface describing a class which holds the execution graph nodes.
 */
class ExecutionPlanGraphInterface {
 public:
  virtual ~ExecutionPlanGraphInterface() = default;
  /**
   * @brief Move all of the root nodes.
   * @return Nodes vectors which have no dependencies.
   */
  virtual auto ExportRootNodes() -> std::vector<std::shared_ptr<QueryNode>> = 0;
  /**
   * @brief Check if there are any nodes held.
   * @return Boolean flag noting if there are any nodes.
   */
  virtual auto IsEmpty() -> bool = 0;
  /**
   * @brief Get non owning ptrs to the root nodes.
   * @return Nodes which have no dependencies.
   */
  virtual auto GetRootNodesPtrs() -> std::vector<QueryNode*> = 0;
  /**
   * @brief Method to return all node pointers
   * @return Vector of all nodes.
   */
  virtual auto GetAllNodesPtrs() -> std::vector<QueryNode*> = 0;
};
}  // namespace orkhestrafs::core_interfaces
