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

#include "plan_evaluator.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <set>
#include <stdexcept>

#include "bitstream_config_helper.hpp"

using orkhestrafs::dbmstodspi::BitstreamConfigHelper;
using orkhestrafs::dbmstodspi::PlanEvaluator;

void PlanEvaluator::FindNewWrittenFrames(
    const std::vector<int>& fully_written_frames,
    std::vector<int>& written_frames,
    const std::vector<ScheduledModule>& configuration) {
  for (const auto& module : configuration) {
    for (int column_i = module.position.first;
         column_i < module.position.second + 1; column_i++) {
      written_frames[column_i] = fully_written_frames.at(column_i);
    }
  }
}

auto PlanEvaluator::FindConfigWrittenForConfiguration(
    const std::vector<ScheduledModule>& current_run,
    const std::vector<ScheduledModule>& previous_configuration,
    const std::string resource_string,
    const std::map<char, int>& cost_of_columns)
    -> std::pair<int, std::vector<ScheduledModule>> {
  std::vector<int> written_frames(resource_string.size(), 0);
  std::vector<int> fully_written_frames;
  for (const auto& column : resource_string) {
    fully_written_frames.push_back(cost_of_columns.at(column));
  }
  auto [reduced_next_config, reduced_current_config] =
      BitstreamConfigHelper::GetConfigCompliments(current_run,
                                                  previous_configuration);

  auto left_over_config = BitstreamConfigHelper::GetResultingConfig(
      previous_configuration, reduced_current_config, reduced_next_config);

  FindNewWrittenFrames(fully_written_frames, written_frames,
                       reduced_next_config);
  FindNewWrittenFrames(fully_written_frames, written_frames,
                       reduced_current_config);

  return {std::accumulate(written_frames.begin(), written_frames.end(), 0),
          left_over_config};
}

auto PlanEvaluator::FindConfigWritten(
    const std::vector<std::vector<ScheduledModule>>& all_runs,
    const std::vector<ScheduledModule>& current_configuration,
    const std::string resource_string,
    const std::map<char, int>& cost_of_columns)
    -> std::pair<int, std::vector<ScheduledModule>> {
  auto [overall_config_written, new_config] =
      FindConfigWrittenForConfiguration(all_runs.front(), current_configuration,
                                        resource_string, cost_of_columns);
  for (int run_i = 0; run_i < all_runs.size() - 1; run_i++) {
    auto [config_written, returned_config] = FindConfigWrittenForConfiguration(
        all_runs.at(run_i + 1), new_config, resource_string, cost_of_columns);
    new_config = returned_config;
    overall_config_written += config_written;
  }
  return {overall_config_written, new_config};
}

auto PlanEvaluator::FindFastestPlan(
    const std::vector<int>& data_streamed,
    const std::vector<int>& configuration_data_wirtten, double streaming_speed,
    double configuration_speed) -> int {
  auto current_min_runtime = std::numeric_limits<double>::max();
  auto current_min_runtime_i = 0;
  for (int plan_i = 0; plan_i < data_streamed.size(); plan_i++) {
    auto current_runtime =
        (data_streamed.at(plan_i) / streaming_speed +
         configuration_data_wirtten.at(plan_i) / configuration_speed);
    if (current_runtime < current_min_runtime) {
      current_min_runtime_i = plan_i;
      current_min_runtime = current_runtime;
    }
  }
  return current_min_runtime_i;
}

auto PlanEvaluator::GetBestPlan(
    int min_run_count, const std::vector<ScheduledModule>& last_configuration,
    const std::string resource_string, double utilites_scaler,
    double config_written_scaler, double utility_per_frame_scaler,
    const std::map<std::vector<std::vector<ScheduledModule>>,
                   ExecutionPlanSchedulingData>& plan_metadata,
    const std::map<char, int>& cost_of_columns, double streaming_speed,
    double configuration_speed)
    -> std::tuple<std::vector<std::vector<ScheduledModule>>,
                 std::vector<ScheduledModule>, int, int> {
  std::vector<int> data_streamed;
  std::vector<int> configuration_data_wirtten;
  std::vector<std::vector<ScheduledModule>> last_configurations;

  std::vector<std::vector<std::vector<ScheduledModule>>> all_plans_list;
  for (const auto& [plan, _] : plan_metadata) {
    all_plans_list.push_back(plan);
  }

  for (int plan_i = 0; plan_i < all_plans_list.size(); plan_i++) {
    data_streamed.push_back(
        plan_metadata.at(all_plans_list.at(plan_i)).streamed_data_size);
    auto [config_written, new_config] =
        FindConfigWritten(all_plans_list.at(plan_i), last_configuration,
                          resource_string, cost_of_columns);
    configuration_data_wirtten.push_back(config_written);
    last_configurations.push_back(new_config);
  }
  int max_plan_i = FindFastestPlan(data_streamed, configuration_data_wirtten,
                                   streaming_speed, configuration_speed);
  std::tuple<std::vector<std::vector<ScheduledModule>>,
            std::vector<ScheduledModule>, int ,int>
      best_plan = {all_plans_list.at(max_plan_i),
                   last_configurations.at(max_plan_i),
                   data_streamed.at(max_plan_i),
                   configuration_data_wirtten.at(max_plan_i)};

  if (std::get<0>(best_plan).empty()) {
    throw std::runtime_error("No plan chosen!");
  }
  return best_plan;
}