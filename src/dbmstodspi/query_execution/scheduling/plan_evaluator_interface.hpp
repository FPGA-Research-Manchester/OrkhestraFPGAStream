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
#include <map>

#include "scheduled_module.hpp"
#include "scheduling_data.hpp"

using orkhestrafs::dbmstodspi::ScheduledModule;
using orkhestrafs::dbmstodspi::scheduling_data::ExecutionPlanSchedulingData;

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
   * @param last_configuration Currently configured modules
   * @param resource_string PR region resources
   * @param utilites_scaler How to prioritize utility
   * @param config_written_scaler How to prioritize config data
   * @param utility_per_frame_scaler How to prioritize utility to data ratio
   * @param plan_metadata All plans meta data
   * @param cost_of_columns How expensive each column is
   * @param streaming_speed Current IO speed
   * @param configuration_speed How fast is configuration data streamed
   * @return Best plan with the last configuration
   */
  virtual auto GetBestPlan(
      const std::vector<std::vector<std::vector<ScheduledModule>>>&
          available_plans,
      int min_run_count, const std::vector<ScheduledModule>& last_configuration,
      const std::string resource_string, double utilites_scaler,
      double config_written_scaler, double utility_per_frame_scaler,
      const std::map<std::vector<std::vector<ScheduledModule>>,
                     ExecutionPlanSchedulingData>& plan_metadata,
      const std::map<char, int>& cost_of_columns, double streaming_speed,
      double configuration_speed)
      -> std::pair<std::vector<std::vector<ScheduledModule>>,
                   std::vector<ScheduledModule>> = 0;
};
}  // namespace orkhestrafs::dbmstodspi