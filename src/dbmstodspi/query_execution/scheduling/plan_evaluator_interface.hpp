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

#include <queue>
#include <utility>
#include <vector>

#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface for plan cost evaluation to choose the best plan
 */
class PlanEvaluatorInterface {
 public:
  virtual ~PlanEvaluatorInterface() = default;
  /**
   * @brief Get the best plan from the vector of fitting plans
   * @param Vector of all of the plans consisting of different runs paired with the modules
   * @return Best plan in a form of queue of runs
  */
  virtual auto GetBestPlan(
          std::vector<
              std::vector<std::pair<ConfigurableModulesVector,
                                    std::vector<std::shared_ptr<QueryNode>>>>>)
          -> std::queue<std::pair<ConfigurableModulesVector,
                                  std::vector<std::shared_ptr<QueryNode>>>> = 0;
};
}  // namespace orkhestrafs::dbmstodspi