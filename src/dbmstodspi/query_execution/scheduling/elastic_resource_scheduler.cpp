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
#include <iostream>
#include <limits>
#include <stdexcept>

#include "elastic_scheduling_graph_parser.hpp"
#include "logger.hpp"
#include "scheduling_data.hpp"
#include "time_limit_execption.hpp"

using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

using orkhestrafs::dbmstodspi::ElasticResourceNodeScheduler;
using orkhestrafs::dbmstodspi::ElasticSchedulingGraphParser;
using orkhestrafs::dbmstodspi::TimeLimitException;

using orkhestrafs::core_interfaces::query_scheduling_data::NodeRunData;

void ElasticResourceNodeScheduler::RemoveUnnecessaryTables(
    const std::unordered_map<std::string, SchedulingQueryNode> &graph,
    std::map<std::string, TableMetadata> &tables) {
  //  std::map<std::string, TableMetadata> resulting_tables;
  //  for (const auto &[table_name, table_data] : tables) {
  //    if (std::any_of(graph.begin(), graph.end(), [&](const auto &p) {
  //          return std::find(p.second.data_tables.begin(),
  //                           p.second.data_tables.end(),
  //                           table_name) != p.second.data_tables.end();
  //        })) {
  //      resulting_tables.insert({table_name, table_data});
  //    }
  //  }
  //  tables = resulting_tables;
}

auto ElasticResourceNodeScheduler::CalculateTimeLimit(
    const std::unordered_map<std::string, SchedulingQueryNode> &graph,
    const std::map<std::string, TableMetadata> &data_tables,
    double config_speed, double streaming_speed,
    const std::unordered_map<QueryOperationType, int> &operation_costs)
    -> double {
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
    const std::map<QueryOperationType, OperationPRModules> & /*hw_libary*/)
    -> std::unordered_map<QueryOperationType, int> {
  // Hardcoded for now.
  std::unordered_map<QueryOperationType, int> operation_costs = {
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

auto ElasticResourceNodeScheduler::ScheduleAndGetAllPlans(
    const std::unordered_set<std::string> &starting_nodes,
    const std::unordered_set<std::string> &processed_nodes,
    const std::unordered_map<std::string, SchedulingQueryNode> &graph,
    const std::map<std::string, TableMetadata> &tables, const Config &config)
    -> std::tuple<int,
                  std::map<std::vector<std::vector<ScheduledModule>>,
                           ExecutionPlanSchedulingData>,
                  long long, bool, std::pair<int, int>> {
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

  scheduler_->SetTimeLimit(time_limit);
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  // Configure the runtime_error behaviour
  try {
    scheduler_->PlaceNodesRecursively(
        std::move(starting_nodes), std::move(processed_nodes), std::move(graph),
        {}, {}, tables, {}, {}, 0);
  } catch (TimeLimitException &e) {
    Log(LogLevel::kInfo, "Timeout of " +
                             std::to_string(time_limit_duration_in_seconds) +
                             " seconds hit by the scheduler.");
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  return {-1, scheduler_->GetResultingPlan(),
          std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
              .count(),
          scheduler_->GetTimeoutStatus(), scheduler_->GetStats()};
}

void ElasticResourceNodeScheduler::BenchmarkScheduling(
    const std::unordered_set<std::string> &first_node_names,
    std::unordered_set<std::string> starting_nodes,
    std::unordered_set<std::string> &processed_nodes,
    std::unordered_map<std::string, SchedulingQueryNode> graph,
    AcceleratorLibraryInterface &drivers,
    std::map<std::string, TableMetadata> &tables,
    std::vector<ScheduledModule> &current_configuration, const Config &config,
    std::map<std::string, double> &benchmark_data) {
  Log(LogLevel::kTrace, "Schedule round");
  std::chrono::steady_clock::time_point begin_pre_process =
      std::chrono::steady_clock::now();
  if (!scheduler_) {
    auto heuristic_choices = GetDefaultHeuristics();
    scheduler_ = std::make_unique<ElasticSchedulingGraphParser>(
        config.pr_hw_library, heuristic_choices.at(config.heuristic_choice),
        first_node_names, drivers, config.use_max_runs_cap,
        config.reduce_single_runs);
  }
  // RemoveUnnecessaryTables(graph, tables);
  scheduler_->PreprocessNodes(starting_nodes, processed_nodes, graph, tables);
  std::chrono::steady_clock::time_point end_pre_process =
      std::chrono::steady_clock::now();
  auto pre_process_time = std::chrono::duration_cast<std::chrono::microseconds>(
                              end_pre_process - begin_pre_process)
                              .count();
  auto [min_runs, resulting_plans, scheduling_time, timed_out, stats] =
      ScheduleAndGetAllPlans(starting_nodes, processed_nodes, graph, tables,
                             config);
  std::chrono::steady_clock::time_point begin_cost_eval =
      std::chrono::steady_clock::now();
  // TODO: Make a separate method for non benchmark for performance
  auto [best_plan, new_last_config, data_amount, configuration_amount] =
      plan_evaluator_->GetBestPlan(
          min_runs, current_configuration, config.resource_string,
          config.utilites_scaler, config.config_written_scaler,
          config.utility_per_frame_scaler, resulting_plans,
          config.cost_of_columns, config.streaming_speed,
          config.configuration_speed);
  std::chrono::steady_clock::time_point end_cost_eval =
      std::chrono::steady_clock::now();
  auto cost_eval_time = std::chrono::duration_cast<std::chrono::microseconds>(
                            end_cost_eval - begin_cost_eval)
                            .count();

  auto overall_time = std::chrono::duration_cast<std::chrono::microseconds>(
                          end_cost_eval - begin_pre_process)
                          .count();
  benchmark_data["discarded_placements"] += stats.second;
  benchmark_data["placed_nodes"] += stats.first;
  benchmark_data["plan_count"] += resulting_plans.size();
  std::cout << "plan_count: " << std::to_string(resulting_plans.size())
            << std::endl;
  benchmark_data["pre_process_time"] += pre_process_time;
  // std::cout << "pre_process_time: " << std::to_string(pre_process_time)
  //          << std::endl;
  benchmark_data["schedule_time"] += scheduling_time;
  // std::cout << "schedule_time: " << std::to_string(scheduling_time)
  //          << std::endl;
  benchmark_data["timeout"] += static_cast<double>(timed_out);
  // std::cout << "timeout: " << std::to_string(timed_out) << std::endl;
  benchmark_data["cost_eval_time"] += cost_eval_time;
  // std::cout << "cost_eval_time: " << std::to_string(cost_eval_time)
  //          << std::endl;
  benchmark_data["overall_time"] += overall_time;
  std::cout << "overall_time: " << std::to_string(overall_time) << std::endl;
  benchmark_data["run_count"] += best_plan.size();
  // std::cout << "run_count: " << std::to_string(best_plan.size()) <<
  // std::endl;
  benchmark_data["data_amount"] += data_amount;
  // std::cout << "data_amount: " << std::to_string(data_amount) << std::endl;
  benchmark_data["configuration_amount"] += configuration_amount;
  // std::cout << "configuration_amount: " <<
  // std::to_string(configuration_amount)
  //          << std::endl;
  benchmark_data["schedule_count"] += 1;

  // starting_nodes = resulting_plans.at(best_plan).available_nodes;
  processed_nodes = resulting_plans.at(best_plan).processed_nodes;
  // graph = resulting_plans.at(best_plan).graph;
  // tables = resulting_plans.at(best_plan).tables;

  current_configuration = new_last_config;

  /*for (const auto &run : best_plan) {
    for (const auto &node : run) {
      std::cout << " " << node.node_name << " ";
    }
    std::cout << std::endl;
  }*/
}

auto ElasticResourceNodeScheduler::GetNextSetOfRuns(
    std::vector<QueryNode *> &available_nodes,
    const std::unordered_set<std::string> &first_node_names,
    std::unordered_set<std::string> starting_nodes,
    std::unordered_map<std::string, SchedulingQueryNode> graph,
    AcceleratorLibraryInterface &drivers,
    std::map<std::string, TableMetadata> &tables,
    const std::vector<ScheduledModule> &current_configuration,
    const Config &config, std::unordered_set<std::string> &skipped_nodes)
    -> std::queue<
        std::pair<std::vector<ScheduledModule>, std::vector<QueryNode *>>> {
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  Log(LogLevel::kTrace, "Scheduling preprocessing.");
  // RemoveUnnecessaryTables(graph, tables);

  if (!scheduler_) {
    auto heuristic_choices = GetDefaultHeuristics();
    scheduler_ = std::make_unique<ElasticSchedulingGraphParser>(
        config.pr_hw_library, heuristic_choices.at(config.heuristic_choice),
        first_node_names, drivers, config.use_max_runs_cap,
        config.reduce_single_runs);
  }

  scheduler_->PreprocessNodes(starting_nodes, skipped_nodes, graph, tables);

  auto [min_runs, resulting_plans, scheduling_time, ignored_timeout,
        ignored_stats] = ScheduleAndGetAllPlans(starting_nodes, skipped_nodes,
                                                graph, tables, config);
  Log(LogLevel::kInfo,
      "Main scheduling loop time = " + std::to_string(scheduling_time / 1000) +
          "[milliseconds]");
  std::cout << "PLAN COUNT:" << resulting_plans.size() << std::endl;

  Log(LogLevel::kTrace, "Choosing best plan.");
  // resulting_plans
  auto [best_plan, new_last_config, ignored_data_size, ignored_config_size] =
      plan_evaluator_->GetBestPlan(
          min_runs, current_configuration, config.resource_string,
          config.utilites_scaler, config.config_written_scaler,
          config.utility_per_frame_scaler, resulting_plans,
          config.cost_of_columns, config.streaming_speed,
          config.configuration_speed);
  Log(LogLevel::kTrace, "Creating module queue.");
  // No need to update the following commented out values
  // starting_nodes = resulting_plans.at(best_plan).available_nodes
  // graph = resulting_plans.at(best_plan).graph;
  // We want skipped nodes for deleting them from the main Graph later
  skipped_nodes = resulting_plans.at(best_plan).processed_nodes;
  // The nodes that aren't in the graph aren't executed anyway and the tables
  // have already been handled.
  for (const auto &[node_name, parameters] : graph) {
    if (skipped_nodes.find(node_name) != skipped_nodes.end()) {
      parameters.node_ptr->is_finished = true;
    }
  }
  // To update tables as normal execution doesn't update sorted statuses.
  tables = resulting_plans.at(best_plan).data_tables;

  auto resulting_runs =
      GetQueueOfResultingRuns(available_nodes, best_plan, config.pr_hw_library);
  // available_nodes = FindNewAvailableNodes(starting_nodes, available_nodes);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "TOTAL SCHEDULING:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << std::endl;
  Log(LogLevel::kTrace, "Execution plan made!");

  /*for (const auto &run : best_plan) {
    for (const auto &node : run) {
      std::cout << " " << node.node_name << " ";
    }
    std::cout << std::endl;
  }*/

  return resulting_runs;
}

void ElasticResourceNodeScheduler::BuildInitialSequencesForMergeSorter(
    std::map<int, LengthOfSortedSequences> &map_of_sequences) {
    // TODO: implement this!
}

// Method to get node pointers to runs from node names and set composed module
// op params! Only thing missing is the resource elastic information. (Like
// merge sort size)
auto ElasticResourceNodeScheduler::GetQueueOfResultingRuns(
    std::vector<QueryNode *> &available_nodes,
    const std::vector<std::vector<ScheduledModule>> &best_plan,
    const std::map<QueryOperationType, OperationPRModules> &hw_library)
    -> std::queue<
        std::pair<std::vector<ScheduledModule>, std::vector<QueryNode *>>> {
  // Clear potentially unfinished nodes
  for (const auto &node : available_nodes) {
    node->module_run_data.clear();
    if (node->operation_type == QueryOperationType::kMergeSort) {
      node->given_operation_parameters.operation_parameters.clear();
    }
  }

  std::unordered_map<std::string, QueryNode *> found_nodes;
  
  std::map<std::string, std::map<int, LengthOfSortedSequences>>
      sorted_status_of_a_merge_node;

  std::queue<std::pair<std::vector<ScheduledModule>, std::vector<QueryNode *>>>
      resulting_runs;
  for (const auto &run : best_plan) {
    std::unordered_map<std::string, NodeRunData> current_run_node_data;

    std::vector<QueryNode *> chosen_nodes;
    for (int module_index = 0; module_index < run.size(); module_index++) {
      QueryNode *chosen_node;
      NodeRunData *current_run_data;
      const auto &current_module = run.at(module_index);
      const auto &node_name = current_module.node_name;
      // Check if we have the corrseponding node found before.
      if (found_nodes.find(node_name) != found_nodes.end()) {
        chosen_node = found_nodes.at(node_name);
      } else {
        found_nodes.insert(
            {node_name, GetNodePointerWithName(available_nodes, node_name)});
      }
      // Check if it is the first occurence of this node in this run
      if (current_run_node_data.find(node_name) !=
          current_run_node_data.end()) {
        current_run_data = &current_run_node_data.at(node_name);

        chosen_node->given_operation_parameters.operation_parameters.back()
            .push_back(hw_library.at(current_module.operation_type)
                           .bitstream_map.at(current_module.bitstream)
                           .capacity.front());
      } else {
        // Chosen nodes already selected here. The NodeRunData will be moved in
        // the end.
        current_run_node_data.insert({node_name, NodeRunData()});
        current_run_data = &current_run_node_data.at(node_name);
        chosen_nodes.push_back(chosen_node);

        // TODO: Remove code duplication
        // Are there any nodes placed afterwards?
        for (int stream_id = 0; stream_id < chosen_node->next_nodes.size();
             stream_id++) {
          auto &after_node = chosen_node->next_nodes.at(stream_id);
          bool is_io_stream = true;
          if (after_node) {
            for (int search_index = module_index + 1; search_index < run.size();
                 search_index++) {
              if (run.at(search_index).node_name == after_node->node_name) {
                is_io_stream = false;
                break;
              }
            }
          }
          if (is_io_stream) {
            current_run_data->output_data_definition_files.push_back(
                chosen_node->given_output_data_definition_files.at(stream_id));
          } else {
            current_run_data->output_data_definition_files.emplace_back("");
          }
        }
        // We assume merge sort is always first currently. Deal with it
        // separately.
        if (chosen_node->operation_type != QueryOperationType::kMergeSort) {
          // Are there any nodes placed before?
          for (int stream_id = 0;
               stream_id < chosen_node->previous_nodes.size(); stream_id++) {
            auto &before_node = chosen_node->next_nodes.at(stream_id);
            bool is_io_stream = true;
            if (before_node) {
              for (int search_index = module_index - 1; search_index >= 0;
                   search_index--) {
                if (run.at(search_index).node_name == before_node->node_name) {
                  is_io_stream = false;
                  break;
                }
              }
            }
            if (is_io_stream) {
              current_run_data->input_data_definition_files.push_back(
                  chosen_node->given_input_data_definition_files.at(stream_id));
            } else {
              current_run_data->input_data_definition_files.emplace_back("");
            }
          }
        } else {
          // First time seeing merge sort in this run. -> Let's check if it has
          // been noticed before?
          if (sorted_status_of_a_merge_node.find(node_name) ==
              sorted_status_of_a_merge_node.end()) {
            sorted_status_of_a_merge_node.insert({node_name, {}});
            BuildInitialSequencesForMergeSorter(
                sorted_status_of_a_merge_node.at(node_name));
          }
          chosen_node->given_operation_parameters.operation_parameters
              .push_back({hw_library.at(current_module.operation_type)
                              .bitstream_map.at(current_module.bitstream)
                              .capacity.front()});
        }
      }
      current_run_data->module_locations.push_back(module_index);
    }
    // TODO:Update Merge sort stuff.
    for (auto &[node_name, run_data] : current_run_node_data) {
      found_nodes.at(node_name)->module_run_data.push_back(std::move(run_data));
    }
    resulting_runs.push({run, chosen_nodes});
  }
  return resulting_runs;
}

auto ElasticResourceNodeScheduler::FindNewAvailableNodes(
    std::unordered_set<std::string> &starting_nodes,
    std::vector<QueryNode *> &available_nodes) -> std::vector<QueryNode *> {
  std::vector<QueryNode *> new_available_nodes;
  for (const auto &previous_start_node : available_nodes) {
    previous_start_node->is_finished = true;
  }
  for (const auto &node_name : starting_nodes) {
    auto chosen_node = orkhestrafs::dbmstodspi::ElasticResourceNodeScheduler::
        GetNodePointerWithName(available_nodes, node_name);
    chosen_node->is_finished = false;
    new_available_nodes.push_back(chosen_node);
  }
  return new_available_nodes;
}

auto ElasticResourceNodeScheduler::GetNodePointerWithName(
    std::vector<QueryNode *> &available_nodes, const std::string &node_name)
    -> QueryNode * {
  QueryNode *chosen_node;
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
      {ModuleSelection(static_cast<std::string>("SHORTEST_AVAILABLE")),
       ModuleSelection(static_cast<std::string>("FIRST_AVAILABLE"))});
  longest_first_module_heuristics.push_back(
      {ModuleSelection(static_cast<std::string>("LONGEST_AVAILABLE")),
       ModuleSelection(static_cast<std::string>("FIRST_AVAILABLE"))});
  shortest_module_heuristics.push_back(
      {ModuleSelection(static_cast<std::string>("SHORTEST_AVAILABLE"))});
  longest_module_heuristics.push_back(
      {ModuleSelection(static_cast<std::string>("LONGEST_AVAILABLE"))});
  all_modules_heuristics.push_back(
      {ModuleSelection(static_cast<std::string>("ALL_AVAILABLE"))});
  return {
      {shortest_first_module_heuristics, longest_first_module_heuristics},
      {shortest_module_heuristics, longest_module_heuristics},
      {shortest_module_heuristics, longest_module_heuristics},
      {all_modules_heuristics, all_modules_heuristics},
      {{}, all_modules_heuristics},
  };
}

auto ElasticResourceNodeScheduler::FindSharedPointerFromRootNodes(
    const std::string &searched_node_name, QueryNode *current_node)
    -> QueryNode * {
  if (current_node->node_name == searched_node_name) {
    return current_node;
  }
  for (const auto &next_node : current_node->next_nodes) {
    if (next_node) {
      auto result =
          FindSharedPointerFromRootNodes(searched_node_name, next_node);
      if (result != nullptr) {
        return result;
      }
    }
  }

  return nullptr;
}
