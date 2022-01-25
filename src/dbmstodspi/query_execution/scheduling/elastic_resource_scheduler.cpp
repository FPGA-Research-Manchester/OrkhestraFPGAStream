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

auto ElasticResourceNodeScheduler::FindAcceleratedQueryNodeSets(
    std::vector<std::shared_ptr<QueryNode>> starting_nodes,
    const std::map<ConfigurableModulesVector, std::string>
        &supported_accelerator_bitstreams,
    const std::map<QueryOperationType, std::vector<std::vector<int>>>
        &existing_modules_library,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>
        &linked_nodes)
    -> std::queue<std::pair<ConfigurableModulesVector,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  throw std::runtime_error("Not implemented!");
  return {};
}

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

auto ElasticResourceNodeScheduler::GetNextSetOfRuns(
    std::vector<std::shared_ptr<QueryNode>> &available_nodes,
    const std::map<QueryOperationType, OperationPRModules> &hw_library,
    const std::vector<std::string> &first_node_names,
    std::vector<std::string> &starting_nodes,
    std::vector<std::string> &processed_nodes,
    std::map<std::string, SchedulingQueryNode> &graph,
    AcceleratorLibraryInterface &drivers,
    std::map<std::string, TableMetadata> &tables)
    -> std::queue<std::pair<ConfigurableModulesVector,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  Log(LogLevel::kTrace, "Scheduling preprocessing.");
  RemoveUnnecessaryTables(graph, tables);

  ElasticSchedulingGraphParser::PreprocessNodes(
      starting_nodes, hw_library, processed_nodes, graph, tables, drivers);
  Log(LogLevel::kTrace, "Starting main scheduling loop.");
  std::map<std::vector<std::vector<ScheduledModule>>,
           ExecutionPlanSchedulingData>
      resulting_plans;
  int min_runs = std::numeric_limits<int>::max();
  std::pair<int, int> placed_nodes_and_discarded_placements = {0, 0};

  auto trigger_timeout = false;
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
  std::vector<std::pair<std::vector<std::vector<ModuleSelection>>,
                        std::vector<std::vector<ModuleSelection>>>>
      heuristic_choices = {
          {shortest_first_module_heuristics, longest_first_module_heuristics},
          {shortest_module_heuristics, longest_module_heuristics},
          {all_modules_heuristics, all_modules_heuristics},
          {{}, all_modules_heuristics}};

  // Have to get from config or calculate
  double time_limit_duration_in_seconds = 3.0;
  bool reduce_single_runs = true;
  bool use_max_runs_cap = true;
  int heuristic_choice = 0;

  auto time_limit =
      std::chrono::system_clock::now() +
      std::chrono::milliseconds(int(time_limit_duration_in_seconds * 1000));

  try {
    ElasticSchedulingGraphParser::PlaceNodesRecursively(
        std::move(starting_nodes), std::move(processed_nodes), std::move(graph),
        {}, {}, resulting_plans, reduce_single_runs, hw_library, min_runs,
        tables, heuristic_choices.at(heuristic_choice),
        placed_nodes_and_discarded_placements, first_node_names, {}, {},
        drivers, time_limit, trigger_timeout, use_max_runs_cap, 0);
  } catch (std::runtime_error &e) {
    Log(LogLevel::kInfo, "Timeout of " +
                             std::to_string(time_limit_duration_in_seconds) +
                             " seconds hit by the scheduler.");
  }

  Log(LogLevel::kTrace, "Choosing best plan.");
  std::vector<std::vector<std::vector<ScheduledModule>>> all_plans;
  for (const auto &[plan, _] : resulting_plans) {
    all_plans.push_back(plan);
  }

  std::vector<ScheduledModule> last_configuration = {};
  std::string resource_string = "MMDMDBMMDBMMDMDBMMDBMMDMDBMMDBM";
  // Not used actually
  double utilites_scaler = 1;
  double config_written_scaler = 1;
  double utility_per_frame_scaler = 1;
  // resulting_plans
  int frame_size = 372;
  std::map<char, int> cost_of_columns = {{'M', 216 * frame_size},
                                         {'D', 200 * frame_size},
                                         {'B', 196 * frame_size}};
  double streaming_speed = 4800000000;
  double configuration_speed = 66000000;

  // TODO: Save new_last_config
  auto [best_plan, new_last_config] = plan_evaluator_->GetBestPlan(
      all_plans, min_runs, last_configuration, resource_string, utilites_scaler,
      config_written_scaler, utility_per_frame_scaler, resulting_plans,
      cost_of_columns, streaming_speed, configuration_speed);
  Log(LogLevel::kTrace, "Creating module queue.");
  starting_nodes = resulting_plans.at(best_plan).available_nodes;
  processed_nodes = resulting_plans.at(best_plan).processed_nodes;
  graph = resulting_plans.at(best_plan).graph;
  tables = resulting_plans.at(best_plan).tables;

  // TODO: move queue construction and available node modification to separate
  // methods
  std::queue<std::pair<ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>
      resulting_runs;
  for (const auto &run : best_plan) {
    ConfigurableModulesVector chosen_modules;
    std::vector<std::shared_ptr<QueryNode>> chosen_nodes;
    for (int module_index = 0; module_index < run.size(); module_index++) {
      // for (const auto &chosen_module : run) {
      chosen_modules.emplace_back(
          run.at(module_index).operation_type,
          hw_library.at(run.at(module_index).operation_type)
              .bitstream_map.at(run.at(module_index).bitstream)
              .capacity);

      std::shared_ptr<QueryNode> chosen_node;
      for (const auto &node : available_nodes) {
        chosen_node = FindSharedPointerFromRootNodes(
            run.at(module_index).node_name, node);
        if (chosen_node != nullptr) {
          break;
        }
      }
      if (chosen_node == nullptr) {
        throw std::runtime_error("No corresponding node found!");
      }
      chosen_node->module_locations.push_back(module_index + 1);
      if (std::find(chosen_nodes.begin(), chosen_nodes.end(), chosen_node) ==
          chosen_nodes.end()) {
        chosen_nodes.push_back(chosen_node);
      }
    }
    resulting_runs.push({chosen_modules, chosen_nodes});
  }

  // TODO: Code duplication here!
  std::vector<std::shared_ptr<QueryNode>> new_available_nodes;
  for (const auto &node_name : starting_nodes) {
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
    new_available_nodes.push_back(chosen_node);
  }
  Log(LogLevel::kTrace, "Execution plan made!");
  available_nodes = new_available_nodes;
  return resulting_runs;
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
