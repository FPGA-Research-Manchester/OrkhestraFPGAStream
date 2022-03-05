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

#include "elastic_resource_scheduler.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>

#include "elastic_scheduling_graph_parser.hpp"
#include "logger.hpp"
#include "scheduling_data.hpp"

using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

using orkhestrafs::dbmstodspi::ElasticResourceNodeScheduler;
using orkhestrafs::dbmstodspi::ElasticSchedulingGraphParser;

void ElasticResourceNodeScheduler::RemoveUnnecessaryTables(
    const std::map<std::string, SchedulingQueryNode> &graph,
    std::map<std::string, TableMetadata> &tables) {
  std::map<std::string, TableMetadata> resulting_tables;
  for (const auto &[table_name, table_data] : tables) {
    if (std::any_of(graph.begin(), graph.end(), [&](const auto &p) {
          return std::find(p.second.data_tables.begin(),
                           p.second.data_tables.end(),
                           table_name) != p.second.data_tables.end();
        })) {
      resulting_tables.insert({table_name, table_data});
    }
  }
  tables = resulting_tables;
}

auto ElasticResourceNodeScheduler::CalculateTimeLimit(
    const std::map<std::string, SchedulingQueryNode> &graph,
    const std::map<std::string, TableMetadata> &data_tables,
    double config_speed, double streaming_speed,
    const std::map<QueryOperationType, int> &operation_costs) -> double {
  int smallest_config_size = 0;
  for (const auto &[node_name, parameters] : graph) {
    smallest_config_size += operation_costs.at(parameters.operation);
  }
  double config_time = smallest_config_size / config_speed;
  int table_sizes = 0;
  for (const auto &[node_name, parameters] : graph) {
    for (const auto &table_name : parameters.data_tables) {
      if (!table_name.empty()) {
        table_sizes += data_tables.at(table_name).record_count *
                       data_tables.at(table_name).record_size * 4;
      }
    }
  }
  double execution_time = table_sizes / streaming_speed;
  return config_time + execution_time;
}

auto ElasticResourceNodeScheduler::GetLargestModulesSizes(
    const std::map<QueryOperationType, OperationPRModules> &hw_libary)
    -> std::map<QueryOperationType, int> {
  // Hardcoded for now.
  std::map<QueryOperationType, int> operation_costs = {
      {QueryOperationType::kFilter, 315456},
      {QueryOperationType::kLinearSort, 770784},
      {QueryOperationType::kMergeSort, 770784},
      {QueryOperationType::kJoin, 462768},
      {QueryOperationType::kAddition, 315456},
      {QueryOperationType::kMultiplication, 916608},
      {QueryOperationType::kAggregationSum, 229152},
  };
  return operation_costs;
}

auto ElasticResourceNodeScheduler::GetNextSetOfRuns(
    std::vector<std::shared_ptr<QueryNode>> &available_nodes,
    const std::vector<std::string> &first_node_names,
    std::vector<std::string> &starting_nodes,
    std::vector<std::string> &processed_nodes,
    std::map<std::string, SchedulingQueryNode> &graph,
    AcceleratorLibraryInterface &drivers,
    std::map<std::string, TableMetadata> &tables,
    const std::vector<ScheduledModule> &current_configuration,
    const Config &config)
    -> std::queue<std::pair<std::vector<ScheduledModule>,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  Log(LogLevel::kTrace, "Scheduling preprocessing.");
  RemoveUnnecessaryTables(graph, tables);

  ElasticSchedulingGraphParser::PreprocessNodes(
      starting_nodes, config.pr_hw_library, processed_nodes, graph, tables,
      drivers);

  Log(LogLevel::kTrace, "Starting main scheduling loop.");
  std::map<std::vector<std::vector<ScheduledModule>>,
           ExecutionPlanSchedulingData>
      resulting_plans;
  int min_runs = std::numeric_limits<int>::max();
  std::pair<int, int> placed_nodes_and_discarded_placements = {0, 0};
  auto heuristic_choices = GetDefaultHeuristics();

  double time_limit_duration_in_seconds = config.time_limit_duration_in_seconds;
  if (time_limit_duration_in_seconds == -1) {
    auto operation_costs = GetLargestModulesSizes(config.pr_hw_library);
    time_limit_duration_in_seconds =
        CalculateTimeLimit(graph, tables, config.configuration_speed,
                           config.streaming_speed, operation_costs);
  }
  auto time_limit =
      std::chrono::system_clock::now() +
      std::chrono::milliseconds(int(time_limit_duration_in_seconds * 1000));
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  bool timeout_trigger = false;

  try {
    ElasticSchedulingGraphParser::PlaceNodesRecursively(
        std::move(starting_nodes), std::move(processed_nodes), std::move(graph),
        {}, {}, resulting_plans, config.reduce_single_runs,
        config.pr_hw_library, min_runs, tables,
        heuristic_choices.at(config.heuristic_choice),
        placed_nodes_and_discarded_placements, first_node_names, {}, {},
        drivers, time_limit, timeout_trigger, config.use_max_runs_cap, 0);
  } catch (std::runtime_error &e) {
    Log(LogLevel::kInfo, "Timeout of " +
                             std::to_string(time_limit_duration_in_seconds) +
                             " seconds hit by the scheduler.");
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  Log(LogLevel::kInfo,
      "Main scheduling loop time = " +
          std::to_string(
              std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[milliseconds]");

  Log(LogLevel::kTrace, "Choosing best plan.");
  std::vector<std::vector<std::vector<ScheduledModule>>> all_plans;
  for (const auto &[plan, _] : resulting_plans) {
    all_plans.push_back(plan);
  }

  // resulting_plans
  // TODO: new_last_config not really needed
  auto [best_plan, new_last_config] = plan_evaluator_->GetBestPlan(
      all_plans, min_runs, current_configuration, config.resource_string,
      config.utilites_scaler, config.config_written_scaler,
      config.utility_per_frame_scaler, resulting_plans, config.cost_of_columns,
      config.streaming_speed, config.configuration_speed);
  Log(LogLevel::kTrace, "Creating module queue.");
  starting_nodes = resulting_plans.at(best_plan).available_nodes;
  processed_nodes = resulting_plans.at(best_plan).processed_nodes;
  graph = resulting_plans.at(best_plan).graph;
  tables = resulting_plans.at(best_plan).tables;

  auto resulting_runs = GetQueueOfResultingRuns(available_nodes, best_plan);

  available_nodes = FindNewAvailableNodes(starting_nodes, available_nodes);
  Log(LogLevel::kTrace, "Execution plan made!");
  return resulting_runs;
}

auto ElasticResourceNodeScheduler::GetQueueOfResultingRuns(
    std::vector<std::shared_ptr<QueryNode>> &available_nodes,
    std::vector<std::vector<ScheduledModule>> best_plan)
    -> std::queue<std::pair<std::vector<ScheduledModule>,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  std::queue<std::pair<std::vector<ScheduledModule>,
                       std::vector<std::shared_ptr<QueryNode>>>>
      resulting_runs;
  for (const auto &run : best_plan) {
    std::vector<std::shared_ptr<QueryNode>> chosen_nodes;
    for (int module_index = 0; module_index < run.size(); module_index++) {
      auto chosen_node = this->GetNodePointerWithName(
          available_nodes, run.at(module_index).node_name);
      chosen_node->module_locations.push_back(module_index + 1);
      if (std::find(chosen_nodes.begin(), chosen_nodes.end(), chosen_node) ==
          chosen_nodes.end()) {
        chosen_nodes.push_back(chosen_node);
      }
    }
    resulting_runs.push({run, chosen_nodes});
  }
  return resulting_runs;
}

auto ElasticResourceNodeScheduler::FindNewAvailableNodes(
    std::vector<std::string> &starting_nodes,
    std::vector<std::shared_ptr<QueryNode>> &available_nodes)
    -> std::vector<std::shared_ptr<QueryNode>> {
  std::vector<std::shared_ptr<QueryNode>> new_available_nodes;
  for (const auto &node_name : starting_nodes) {
    auto chosen_node = this->GetNodePointerWithName(available_nodes, node_name);
    new_available_nodes.push_back(chosen_node);
  }
  return new_available_nodes;
}

auto ElasticResourceNodeScheduler::GetNodePointerWithName(
    std::vector<std::shared_ptr<QueryNode>> &available_nodes,
    std::string node_name) -> std::shared_ptr<QueryNode> {
  std::shared_ptr<QueryNode> chosen_node;
  for (const auto &node : available_nodes) {
    chosen_node = FindSharedPointerFromRootNodes(node_name, node);
    if (chosen_node != nullptr) {
      break;
    }
  }
  if (chosen_node == nullptr) {
    throw std::runtime_error("No corresponding node found!");
  }
  return std::move(chosen_node);
}

auto ElasticResourceNodeScheduler::GetDefaultHeuristics()
    -> const std::vector<std::pair<std::vector<std::vector<ModuleSelection>>,
                                   std::vector<std::vector<ModuleSelection>>>> {
  std::vector<std::vector<ModuleSelection>> shortest_first_module_heuristics;
  std::vector<std::vector<ModuleSelection>> longest_first_module_heuristics;
  std::vector<std::vector<ModuleSelection>> shortest_module_heuristics;
  std::vector<std::vector<ModuleSelection>> longest_module_heuristics;
  std::vector<std::vector<ModuleSelection>> all_modules_heuristics;
  shortest_first_module_heuristics.push_back(
      {static_cast<std::string>("SHORTEST_AVAILABLE"),
       static_cast<std::string>("FIRST_AVAILABLE")});
  longest_first_module_heuristics.push_back(
      {static_cast<std::string>("LONGEST_AVAILABLE"),
       static_cast<std::string>("FIRST_AVAILABLE")});
  shortest_module_heuristics.push_back(
      {static_cast<std::string>("SHORTEST_AVAILABLE")});
  longest_module_heuristics.push_back(
      {static_cast<std::string>("LONGEST_AVAILABLE")});
  all_modules_heuristics.push_back({static_cast<std::string>("ALL_AVAILABLE")});
  return {{shortest_first_module_heuristics, longest_first_module_heuristics},
          {shortest_module_heuristics, longest_module_heuristics},
          {all_modules_heuristics, all_modules_heuristics},
          {{}, all_modules_heuristics}};
}

auto ElasticResourceNodeScheduler::FindSharedPointerFromRootNodes(
    std::string searched_node_name, std::shared_ptr<QueryNode> current_node)
    -> std::shared_ptr<QueryNode> {
  if (current_node->node_name == searched_node_name) {
    return current_node;
  } else {
    for (const auto &next_node : current_node->next_nodes) {
      auto result =
          FindSharedPointerFromRootNodes(searched_node_name, next_node);
      if (result != nullptr) {
        return result;
      }
    }
  }
  return nullptr;
}
