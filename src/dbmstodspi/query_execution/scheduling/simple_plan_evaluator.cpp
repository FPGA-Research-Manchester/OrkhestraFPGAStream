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

#include "simple_plan_evaluator.hpp"

#include <set>
#include <stdexcept>

using orkhestrafs::dbmstodspi::SimplePlanEvaluator;

// Naive best plan chooser where just simply the plan with min runs, max nodes
// and min configuration overhead is chosen
// (TODO:module reuse isn't considered)
auto SimplePlanEvaluator::GetBestPlan(
    int min_run_count, const std::vector<ScheduledModule>& last_configuration,
    const std::string resource_string, double utilites_scaler,
    double config_written_scaler, double utility_per_frame_scaler,
    const std::map<std::vector<std::vector<ScheduledModule>>,
                   ExecutionPlanSchedulingData>& plan_metadata,
    const std::map<char, int>& cost_of_columns, double streaming_speed,
    double configuration_speed)
    -> std::pair<std::vector<std::vector<ScheduledModule>>,
                 std::vector<ScheduledModule>> {
  std::pair<std::vector<std::vector<ScheduledModule>>,
            std::vector<ScheduledModule>>
      best_plan;
  int max_nodes_in_min_plan = 0;
  int min_configured_rows_in_min_plan = 0;

  std::vector<std::vector<std::vector<ScheduledModule>>> all_plans_list;
  for (const auto& [plan, _] : plan_metadata) {
    all_plans_list.push_back(plan);
  }

  for (int plan_index = 0; plan_index < all_plans_list.size(); plan_index++) {
    if (all_plans_list.at(plan_index).size() == min_run_count) {
      int configured_rows = 0;
      std::set<std::string> unique_node_names;
      for (int run_index = 0; run_index < all_plans_list.at(plan_index).size();
           run_index++) {
        for (const auto& chosen_module :
             all_plans_list.at(plan_index).at(run_index)) {
          unique_node_names.insert(chosen_module.node_name);
          configured_rows +=
              chosen_module.position.second - chosen_module.position.first + 1;
        }
        if (unique_node_names.size() > max_nodes_in_min_plan) {
          max_nodes_in_min_plan = unique_node_names.size();
          min_configured_rows_in_min_plan = configured_rows;
          best_plan = {all_plans_list.at(plan_index), {}};
        } else if (unique_node_names.size() == max_nodes_in_min_plan &&
                   configured_rows < min_configured_rows_in_min_plan) {
          min_configured_rows_in_min_plan = configured_rows;
          best_plan = {all_plans_list.at(plan_index), {}};
        }
      }
    }
  }
  if (best_plan.first.empty()) {
    throw std::runtime_error("No plan chosen!");
  }
  return best_plan;
}