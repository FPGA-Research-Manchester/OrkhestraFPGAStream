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
    const std::vector<ScheduledModule>& next_config,
    const std::vector<ScheduledModule>& current_config,
    std::vector<std::string>& current_routing,
    const std::map<std::string, int>& cost_of_bitstreams)
    -> std::pair<int, std::vector<ScheduledModule>> {

  std::vector<int> written_frames(31, 0);

  auto old_routing_modules = BitstreamConfigHelper::GetOldNonOverlappingModules(
      next_config, current_config);

  auto [reduced_next_config, reduced_current_config] =
      BitstreamConfigHelper::GetConfigCompliments(next_config, current_config);

  auto removable_modules = reduced_current_config;
  for (const auto& reused_module : old_routing_modules) {
    for (const auto& removable_module : reduced_current_config) {
      removable_modules.erase(
          std::remove(removable_modules.begin(), removable_modules.end(),
                      reused_module),
          removable_modules.end());
    }
  }

  // Find out which frames need writing to.
  for (const auto& module : removable_modules) {
    for (int column_i = module.position.first;
         column_i < module.position.second + 1; column_i++) {
      written_frames[column_i] = 1;
      current_routing[column_i] = "";
    }
  }

  std::vector<std::string> required_bitstreams;
  for (const auto& new_module : reduced_next_config) {
    for (int column_i = new_module.position.first;
         column_i < new_module.position.second + 1; column_i++) {
      written_frames[column_i] = 0;
      current_routing[column_i] = new_module.bitstream;
    }
    required_bitstreams.push_back(new_module.bitstream);
  }

  auto left_over_config = BitstreamConfigHelper::GetResultingConfig(
      current_config, removable_modules, reduced_next_config);
  std::sort(left_over_config.begin(), left_over_config.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.position.first < rhs.position.first;
            });

  std::vector<std::pair<QueryOperationType, bool>> passthrough_modules;

  // 1.Find out how far does it have to go
  int furthest_required_column = next_config.back().position.second;
  // 2.Check if there is a connection from beginning - if not add RT
  // And find all passthrough modules
  std::unordered_map<std::string, QueryOperationType> old_operations;
  for (const auto& prev_module : current_config) {
    old_operations.insert({prev_module.bitstream, prev_module.operation_type});
  }
  std::unordered_map<std::string, QueryOperationType> new_operations;
  for (const auto& new_module : next_config) {
    new_operations.insert({new_module.bitstream, new_module.operation_type});
  }

  std::string last_seen_bitstream = "";
  for (int column_i = 0; column_i <= furthest_required_column; column_i++) {
    if (current_routing[column_i].empty() ||
        current_routing[column_i] == "TAA") {
      current_routing[column_i] = "RT";
      required_bitstreams.push_back("RT");
    } else if (last_seen_bitstream == current_routing[column_i] ||
               current_routing[column_i] == "RT") {
      // Do nothing
    } else if (last_seen_bitstream != current_routing[column_i] &&
               new_operations.find(current_routing[column_i]) !=
                   new_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
    } else if (last_seen_bitstream != current_routing[column_i] &&
               old_operations.find(current_routing[column_i]) !=
                   old_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
    } else {
      throw std::runtime_error("Unknown routing bitstream");
    }
  }
  // 3.Check if there is a connection to TAA or end from the end - if not add
  // TAA
  last_seen_bitstream = "";
  for (int column_i = furthest_required_column + 1;
       column_i < 31; column_i++) {
    if (current_routing[column_i].empty()) {
      current_routing[column_i] = "TAA";
      required_bitstreams.push_back("RT");
      break;
    } else if (current_routing[column_i] == "TAA") {
      break;
    } else if (last_seen_bitstream == current_routing[column_i] ||
               current_routing[column_i] == "RT") {
      // Do nothing
    } else if (last_seen_bitstream != current_routing[column_i] &&
               new_operations.find(current_routing[column_i]) !=
                   new_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
    } else if (last_seen_bitstream != current_routing[column_i] &&
               old_operations.find(current_routing[column_i]) !=
                   old_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
    } else {
      throw std::runtime_error("Unknown routing bitstream");
    }
  }

  int cost = 0;
  for (const auto& bitstream : required_bitstreams){
    cost+=cost_of_bitstreams.at(bitstream);
  }

  return {cost,
          std::move(left_over_config)};
}

auto PlanEvaluator::FindConfigWritten(
    const std::vector<std::vector<ScheduledModule>>& all_runs,
    const std::vector<ScheduledModule>& current_configuration,
    const std::map<std::string, int>& cost_of_bitstreams)
    -> std::pair<int, std::vector<ScheduledModule>> {
  std::vector<std::string> current_columns;
  for (int i = 0; i<31; i++){
    current_columns.push_back("RT");
  }
  for (const auto& current_module: current_configuration){
    for (int i = current_module.position.first; i<= current_module.position.second; i++){
      current_columns[i] = current_module.bitstream;
    }
  }
  auto [overall_config_written, new_config] =
      FindConfigWrittenForConfiguration(all_runs.front(), current_configuration,
                                        current_columns, cost_of_bitstreams);
  for (int run_i = 0; run_i < all_runs.size() - 1; run_i++) {
    auto [config_written, returned_config] = FindConfigWrittenForConfiguration(
        all_runs.at(run_i + 1), new_config, current_columns, cost_of_bitstreams);
    new_config = returned_config;
    overall_config_written += config_written;
  }
  return {overall_config_written, new_config};
}

auto PlanEvaluator::FindFastestPlan(
    const std::vector<long>& data_streamed,
    const std::vector<long>& configuration_time, double streaming_speed) -> int {
  auto current_min_runtime = std::numeric_limits<double>::max();
  auto current_min_runtime_i = 0;
  for (int plan_i = 0; plan_i < data_streamed.size(); plan_i++) {
    auto current_runtime =
        (data_streamed.at(plan_i) / streaming_speed +
         configuration_time.at(plan_i));
    if (current_runtime < current_min_runtime) {
      current_min_runtime_i = plan_i;
      current_min_runtime = current_runtime;
    }
  }
  return current_min_runtime_i;
}

auto PlanEvaluator::GetBestPlan(
    int /*min_run_count*/,
    const std::vector<ScheduledModule>& last_configuration,
    const std::string resource_string, double /*utilites_scaler*/,
    double /*config_written_scaler*/, double /*utility_per_frame_scaler*/,
    const std::map<std::vector<std::vector<ScheduledModule>>,
                   ExecutionPlanSchedulingData>& plan_metadata,
    const std::map<char, int>& cost_of_columns, double streaming_speed,
    double /*configuration_speed*/)
    -> std::tuple<std::vector<std::vector<ScheduledModule>>,
                  std::vector<ScheduledModule>, long, long> {
  std::vector<long> data_streamed;
  std::vector<long> configuration_times;
  std::vector<std::vector<ScheduledModule>> last_configurations;

  std::vector<std::vector<std::vector<ScheduledModule>>> all_plans_list;
  all_plans_list.reserve(plan_metadata.size());
  for (const auto& [plan, _] : plan_metadata) {
    all_plans_list.push_back(plan);
  }

  for (auto& plan_i : all_plans_list) {
    data_streamed.push_back(plan_metadata.at(plan_i).streamed_data_size);
    auto [config_time, new_config] = FindConfigWritten(
        plan_i, last_configuration, cost_of_bitstreams_);
    configuration_times.push_back(config_time);
    last_configurations.push_back(new_config);
  }
  int max_plan_i = FindFastestPlan(data_streamed, configuration_times,
                                   streaming_speed);
  std::tuple<std::vector<std::vector<ScheduledModule>>,
             std::vector<ScheduledModule>, long, long>
      best_plan = {all_plans_list.at(max_plan_i),
                   last_configurations.at(max_plan_i),
                   data_streamed.at(max_plan_i),
                   configuration_times.at(max_plan_i)};

  if (std::get<0>(best_plan).empty()) {
    throw std::runtime_error("No plan chosen!");
  }
  return best_plan;
}