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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "execution_plan_graph_interface.hpp"

namespace orkhestrafs::core_interfaces {
/**
 * @brief Interface for a class to implement the execute the given query graph.
 */
class ExecutionManagerInterface {
 public:
  virtual ~ExecutionManagerInterface() = default;
  /**
   * @brief Method to execute the operation nodes in the given graph.
   * @param execution_graph Graph representing operation nodes and their
   * dependencies.
   */
  virtual void Execute(
      std::unique_ptr<ExecutionPlanGraphInterface> execution_graph) = 0;
};
}  // namespace orkhestrafs::core_interfaces