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
#include <numeric>
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
  // std::cout << "configuration_amount: "
  // std::to_string(configuration_amount)
  //          << std::endl;
  benchmark_data["schedule_count"] += 1;

  // starting_nodes = resulting_plans.at(best_plan).available_nodes;
  processed_nodes = resulting_plans.at(best_plan).processed_nodes;
  // graph = resulting_plans.at(best_plan).graph;
  // tables = resulting_plans.at(best_plan).tables;

  current_configuration = new_last_config;

  // TODO: Need to update available nodes for next run!
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
    const Config &config, std::unordered_set<std::string> &skipped_nodes,
    std::unordered_map<std::string, int> &table_counter)
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
  // TODO: Potentially remove this boolean!
  for (const auto &[node_name, parameters] : graph) {
    if (skipped_nodes.find(node_name) != skipped_nodes.end()) {
      parameters.node_ptr->is_finished = true;
    }
  }
  // To update tables as normal execution doesn't update sorted statuses.
  tables = resulting_plans.at(best_plan).data_tables;

  auto resulting_runs = GetQueueOfResultingRuns(
      available_nodes, best_plan, config.pr_hw_library, tables, table_counter);
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

// LengthOfSortedSequences = offset, number_of_sequences, table_name
void ElasticResourceNodeScheduler::BuildInitialSequencesForMergeSorter(
    std::map<int, std::vector<LengthOfSortedSequences>> &map_of_sequences,
    const TableMetadata &table_data, std::string table_name) {
  // We have sorted status
  // 1. Begin of first set of sequences
  // 2. End of the first set of sequences
  // 3. Size of the sequences
  // 4. How many sequences
  const int param_count = 4;
  const int begin_offset = 0;
  const int end_offset = 1;
  const int size_offset = 2;
  const int count_offset = 3;
  const auto &sorted_status = table_data.sorted_status;
  for (int set_of_sequences_id = 0;
       set_of_sequences_id <= sorted_status.size() - param_count;
       set_of_sequences_id += param_count) {
    const int number_of_data_rows =
        sorted_status.at(set_of_sequences_id + end_offset) -
        sorted_status.at(set_of_sequences_id + begin_offset) + 1;
    int number_of_sequences =
        sorted_status.at(set_of_sequences_id + count_offset);
    // IF the last one doesn't have the same size then reduce the number of
    // sequences and add another size.
    if (number_of_sequences *
            sorted_status.at(set_of_sequences_id + size_offset) !=
        number_of_data_rows) {
      number_of_sequences -= 1;
      const int left_over_size =
          number_of_data_rows -
          number_of_sequences *
              sorted_status.at(set_of_sequences_id + size_offset);
      LengthOfSortedSequences new_sequence = {
          sorted_status.at(set_of_sequences_id + begin_offset), 1, table_name};
      auto search = map_of_sequences.find(left_over_size);
      if (search == map_of_sequences.end()) {
        map_of_sequences.insert({left_over_size, {new_sequence}});
      } else {
        search->second.push_back(new_sequence);
      }
    }
    LengthOfSortedSequences new_sequence = {
        sorted_status.at(set_of_sequences_id + begin_offset),
        number_of_sequences, table_name};
    auto search = map_of_sequences.find(
        sorted_status.at(set_of_sequences_id + count_offset));
    if (search == map_of_sequences.end()) {
      map_of_sequences.insert(
          {sorted_status.at(set_of_sequences_id + count_offset),
           {new_sequence}});
    } else {
      search->second.push_back(new_sequence);
    }
  }
}

// Method to get node pointers to runs from node names and set composed module
// op params!
auto ElasticResourceNodeScheduler::GetQueueOfResultingRuns(
    std::vector<QueryNode *> &available_nodes,
    const std::vector<std::vector<ScheduledModule>> &best_plan,
    const std::map<QueryOperationType, OperationPRModules> &hw_library,
    std::map<std::string, TableMetadata> &table_data,
    std::unordered_map<std::string, int> &table_counter)
    -> std::queue<
        std::pair<std::vector<ScheduledModule>, std::vector<QueryNode *>>> {
  // Clear potentially unfinished nodes
  for (const auto &node : available_nodes) {
    node->module_run_data = {};
    // For merge sort it is used to say the module sizes for each run.
    if (node->operation_type == QueryOperationType::kMergeSort) {
      node->given_operation_parameters.operation_parameters.clear();
    }
  }

  std::unordered_map<std::string, QueryNode *> found_nodes;
  std::unordered_map<std::string, int> run_counter;

  std::map<std::string, std::map<int, std::vector<LengthOfSortedSequences>>>
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
      // Check if we have the corresponding node found before.
      if (found_nodes.find(node_name) != found_nodes.end()) {
        chosen_node = found_nodes.at(node_name);
      } else {
        found_nodes.insert(
            {node_name, GetNodePointerWithName(available_nodes, node_name)});
      }
      // Check if it is the first occurrence of this node in this run
      if (current_run_node_data.find(node_name) !=
          current_run_node_data.end()) {
        current_run_data = &current_run_node_data.at(node_name);

        if (chosen_node->operation_type == QueryOperationType::kMergeSort) {
          chosen_node->given_operation_parameters.operation_parameters.back()
              .push_back(hw_library.at(current_module.operation_type)
                             .bitstream_map.at(current_module.bitstream)
                             .capacity.front());
        }

      } else {
        if (const auto &[it, inserted] = run_counter.try_emplace(node_name, 1);
            !inserted) {
          it->second++;
        }

        // Chosen nodes already selected here. The NodeRunData will be moved in
        // the end.
        current_run_node_data.insert({node_name, NodeRunData()});
        current_run_data = &current_run_node_data.at(node_name);
        chosen_nodes.push_back(chosen_node);
        current_run_data->run_index = run_counter.at(node_name) - 1;

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
          current_run_data->output_offset.push_back(0);
          if (is_io_stream) {
            current_run_data->output_data_definition_files.push_back(
                chosen_node->given_output_data_definition_files.at(stream_id));
            if (const auto &[it, inserted] = table_counter.try_emplace(
                    chosen_node->given_output_data_definition_files.at(
                        stream_id),
                    1);
                !inserted) {
              it->second++;
            }
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
              if (const auto &[it, inserted] = table_counter.try_emplace(
                      chosen_node->given_input_data_definition_files.at(
                          stream_id),
                      1);
                  !inserted) {
                it->second++;
              }
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
            // Assuming merge sort has one table given
            const auto &table_name =
                chosen_node->given_input_data_definition_files.front();
            BuildInitialSequencesForMergeSorter(
                sorted_status_of_a_merge_node[node_name],
                table_data.at(table_name), table_name);
          }
          chosen_node->given_operation_parameters.operation_parameters
              .push_back({hw_library.at(current_module.operation_type)
                              .bitstream_map.at(current_module.bitstream)
                              .capacity.front()});
        }
      }
      current_run_data->module_locations.push_back(module_index);
    }
    for (auto &[node_name, run_data] : current_run_node_data) {
      found_nodes.at(node_name)->module_run_data.push_back(std::move(run_data));
    }
    resulting_runs.push({run, chosen_nodes});
  }
  for (auto &[merge_sort_node_name, sort_status] :
       sorted_status_of_a_merge_node) {
    // If run count is 0 or missing - Something went wrong.
    // If the number of parameters or run_data doesn't match the run_count -
    // Something went wrong. Assuming the above is correct!
    auto &sort_node = found_nodes.at(merge_sort_node_name);
    auto &run_data = sort_node->module_run_data;
    auto &parameters =
        sort_node->given_operation_parameters.operation_parameters;
    for (int run_id = 0; run_id < run_counter.at(merge_sort_node_name);
         run_id++) {
      std::vector<int> next_run_capacities;
      // If penultimate run - set next run capabilities as well.
      if (run_id + 2 == run_counter.at(merge_sort_node_name)) {
        next_run_capacities = parameters[run_id + 1];
      }
      UpdateSortedStatusAndRunData(
          sort_status, run_data.at(run_id), parameters[run_id], table_data,
          sort_node, next_run_capacities,
          run_id == (run_counter.at(merge_sort_node_name) - 2), table_counter);
    }
    // Could also check that the number of sorted rows is correct rather than
    // that all sequences have been processed.
    if (!sort_status.empty()) {
      throw std::runtime_error("Not enough sorting modules scheduled!");
    }
  }
  return resulting_runs;
}

auto ElasticResourceNodeScheduler::GetCapacityForPenultimateRun(
    int next_run_capacity,
    const std::map<int, std::vector<LengthOfSortedSequences>> &sort_status)
    -> int {
  // The last run capacity must be equal to
  // current_sequence.number_of_sequences; With all sort statuses.
  int current_sequences = 0;
  for (const auto &[size, sequences] : sort_status) {
    for (const auto &sequence : sequences) {
      current_sequences += sequence.number_of_sequences;
    }
  }

  return current_sequences + 1 - next_run_capacity;
}

void ElasticResourceNodeScheduler::UpdateSortedStatusAndRunData(
    std::map<int, std::vector<LengthOfSortedSequences>> &sort_status,
    NodeRunData &run_data, const std::vector<int> &capacities,
    std::map<std::string, TableMetadata> &table_data, QueryNode *merge_node,
    const std::vector<int> &next_run_capacities, bool is_penultimate,
    std::unordered_map<std::string, int> &table_counter) {
  if (sort_status.empty()) {
    throw std::runtime_error("Too many merge sort modules scheduled!");
  }
  int leftover_capacity = 0;
  if (is_penultimate) {
    leftover_capacity = GetCapacityForPenultimateRun(
        std::accumulate(next_run_capacities.begin(), next_run_capacities.end(),
                        0),
        sort_status);
    if (leftover_capacity >
            std::accumulate(capacities.begin(), capacities.end(), 0) ||
        leftover_capacity <= 0) {
      throw std::runtime_error("Incorrect penultimate capacity!");
    }
  } else {
    leftover_capacity =
        std::accumulate(capacities.begin(), capacities.end(), 0);
  }
  int new_sequence_size = 0;
  while (leftover_capacity != 0 && !sort_status.empty()) {
    int current_sequence_size = sort_status.begin()->first;
    auto &current_sequence = sort_status.begin()->second.back();
    // LengthOfSortedSequences = offset, number_of_sequences, table_name
    int original_leftover_capacity = leftover_capacity;
    int original_number_of_sequences = current_sequence.number_of_sequences;
    for (int i = 0;
         i < original_leftover_capacity && i < original_number_of_sequences;
         i++) {
      auto find_it = std::find(run_data.input_data_definition_files.begin(),
                               run_data.input_data_definition_files.end(),
                               current_sequence.table_name);
      if (find_it == run_data.input_data_definition_files.end()) {
        run_data.input_data_definition_files.push_back(
            current_sequence.table_name);
      }
      int table_index = find_it - run_data.input_data_definition_files.begin();
      new_sequence_size += current_sequence_size;
      run_data.operation_parameters.push_back(
          {table_index, current_sequence.offset, current_sequence_size});

      leftover_capacity--;
      current_sequence.number_of_sequences--;
      current_sequence.offset += current_sequence_size;
    }
    if (current_sequence.number_of_sequences == 0) {
      sort_status.begin()->second.pop_back();
      if (sort_status.begin()->second.empty()) {
        sort_status.erase(current_sequence_size);
      }
    }
  }
  for (const auto &input_table : run_data.input_data_definition_files) {
    if (const auto &[it, inserted] = table_counter.try_emplace(input_table, 1);
        !inserted) {
      it->second++;
    }
  }
  if (!sort_status.empty()) {
    // Else you put a new sequence back and set output.

    // I need a new table name
    int max_iteration = 0;
    for (const auto &input_table_name : run_data.input_data_definition_files) {
      max_iteration = std::max(
          max_iteration,
          static_cast<int>('a') - static_cast<int>(input_table_name.back()));
    }
    auto output_table_name =
        merge_node->given_input_data_definition_files.front();
    output_table_name.pop_back();
    output_table_name += std::to_string(max_iteration + 1);
    // Check if table exists.
    if (table_data.find(output_table_name) == table_data.end()) {
      TableMetadata new_data = {
          table_data.at(merge_node->given_input_data_definition_files.front())
              .record_size,
          0,
          {}};
      new_data.record_count = 0;
      table_data.insert({output_table_name, std::move(new_data)});
    }
    // I also need the size of the table for the offset
    // Since this is merge sort we assume a single output stream
    run_data.output_offset = {table_data.at(output_table_name).record_count};
    // Then check if it exists already or not.
    bool sequence_merged = false;
    if (sort_status.find(new_sequence_size) != sort_status.end()) {
      // If exists check all of the vectors and see if you can just +1 the count
      // of sequences where the table name matches and offset is at the end of
      // the table. Offset required for just +1
      int required_output_offset_for_merge =
          run_data.output_offset.front() - new_sequence_size;
      if (required_output_offset_for_merge > 0) {
        for (auto &sequences : sort_status.at(new_sequence_size)) {
          if (sequences.table_name == output_table_name &&
              sequences.offset == required_output_offset_for_merge) {
            sequences.number_of_sequences++;
            sequence_merged = true;
          }
        }
      }
    }
    if (!sequence_merged) {
      LengthOfSortedSequences new_sequence = {run_data.output_offset.front(), 1,
                                              output_table_name};
      sort_status.insert({new_sequence_size, {std::move(new_sequence)}});
    }
    table_data[output_table_name].record_count += new_sequence_size;
    // Half sorted stuff always gets written to temp file
    table_counter[run_data.output_data_definition_files.front()]--;
    run_data.output_data_definition_files.clear();
    run_data.output_data_definition_files.push_back(output_table_name);
    if (const auto &[it, inserted] =
            table_counter.try_emplace(output_table_name, 1);
        !inserted) {
      it->second++;
    }
  } else {
    // if empty check that all nodes have been sorted!
    if (table_data.at(merge_node->given_input_data_definition_files.front())
            .record_count != new_sequence_size) {
      throw std::runtime_error("Incorrect number of records have been sorted!");
    }
  }
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
