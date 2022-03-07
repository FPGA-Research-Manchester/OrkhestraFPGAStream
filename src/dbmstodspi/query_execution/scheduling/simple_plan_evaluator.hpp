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
#include "plan_evaluator_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to schedule nodes to groups of different FPGA runs.
 */
class SimplePlanEvaluator : public PlanEvaluatorInterface {
 public:
  auto GetBestPlan(int min_run_count,
                   const std::vector<ScheduledModule>& last_configuration,
                   const std::string resource_string, double utilites_scaler,
                   double config_written_scaler,
                   double utility_per_frame_scaler,
                   const std::map<std::vector<std::vector<ScheduledModule>>,
                                  ExecutionPlanSchedulingData>& plan_metadata,
                   const std::map<char, int>& cost_of_columns,
                   double streaming_speed, double configuration_speed)
      -> std::pair<std::vector<std::vector<ScheduledModule>>,
                   std::vector<ScheduledModule>> override;
};

}  // namespace orkhestrafs::dbmstodspi