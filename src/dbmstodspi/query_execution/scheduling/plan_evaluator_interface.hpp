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

#include <vector>

#include "scheduled_module.hpp"

using orkhestrafs::dbmstodspi::ScheduledModule;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface for plan cost evaluation to choose the best plan
 */
class PlanEvaluatorInterface {
 public:
  virtual ~PlanEvaluatorInterface() = default;
  /**
   * @brief Get the best plan from the vector of fitting plans
   * @param available_plans All avaialable plans to choose from.
   * @param min_run_count Minimum run count in a plan.
   * @return Best plan
   */
  virtual auto GetBestPlan(
      std::vector<std::vector<std::vector<ScheduledModule>>> available_plans,
      int min_run_count) -> std::vector<std::vector<ScheduledModule>> = 0;
};
}  // namespace orkhestrafs::dbmstodspi