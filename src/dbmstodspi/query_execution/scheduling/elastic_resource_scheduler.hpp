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

#include <memory>

#include "elastic_scheduling_graph_parser.hpp"
#include "module_selection.hpp"
#include "node_scheduler_interface.hpp"
#include "plan_evaluator_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to schedule nodes to groups of different FPGA runs using module
 * resource information.
 */
class ElasticResourceNodeScheduler : public NodeSchedulerInterface {
 public:
  explicit ElasticResourceNodeScheduler(
      std::unique_ptr<PlanEvaluatorInterface> plan_evaluator)
      : plan_evaluator_{std::move(plan_evaluator)} {}

  auto GetNextSetOfRuns(
      std::vector<std::shared_ptr<QueryNode>> &available_nodes,
      const std::unordered_set<std::string> &first_node_names,
      std::unordered_set<std::string> &starting_nodes,
      std::unordered_set<std::string> &processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode> &graph,
      AcceleratorLibraryInterface &drivers,
      std::map<std::string, TableMetadata> &tables,
      const std::vector<ScheduledModule> &current_configuration,
      const Config &config)
      -> std::queue<
          std::pair<std::vector<ScheduledModule>,
                    std::vector<std::shared_ptr<QueryNode>>>> override;

  auto ScheduleAndGetAllPlans(
      std::unordered_set<std::string> &starting_nodes,
      std::unordered_set<std::string> &processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode> &graph,
      std::map<std::string, TableMetadata> &tables, const Config &config)
      -> std::tuple<int,
                    std::map<std::vector<std::vector<ScheduledModule>>,
                             ExecutionPlanSchedulingData>,
                    long long, bool, std::pair<int, int>> override;

  void BenchmarkScheduling(
      const std::unordered_set<std::string> &first_node_names,
      std::unordered_set<std::string> &starting_nodes,
      std::unordered_set<std::string> &processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode> &graph,
      AcceleratorLibraryInterface &drivers,
      std::map<std::string, TableMetadata> &tables,
      std::vector<ScheduledModule> &current_configuration, const Config &config,
      std::map<std::string, double> &benchmark_data) override;

 private:
  static auto CalculateTimeLimit(
      const std::unordered_map<std::string, SchedulingQueryNode> &graph,
      const std::map<std::string, TableMetadata> &data_tables,
      double config_speed, double streaming_speed,
      const std::unordered_map<QueryOperationType, int> &operation_costs)
      -> double;
  static auto FindSharedPointerFromRootNodes(
      const std::string &searched_node_name,
      std::shared_ptr<QueryNode> current_node) -> std::shared_ptr<QueryNode>;
  static void RemoveUnnecessaryTables(
      const std::unordered_map<std::string, SchedulingQueryNode> &graph,
      std::map<std::string, TableMetadata> &tables);
  static auto GetDefaultHeuristics() -> const
      std::vector<std::pair<std::vector<std::vector<ModuleSelection>>,
                            std::vector<std::vector<ModuleSelection>>>>;
  static auto GetNodePointerWithName(
      std::vector<std::shared_ptr<QueryNode>> &available_nodes,
      const std::string &node_name) -> std::shared_ptr<QueryNode>;
  static auto FindNewAvailableNodes(
      std::unordered_set<std::string> &starting_nodes,
      std::vector<std::shared_ptr<QueryNode>> &available_nodes)
      -> std::vector<std::shared_ptr<QueryNode>>;
  static auto GetQueueOfResultingRuns(
      std::vector<std::shared_ptr<QueryNode>> &available_nodes,
      const std::vector<std::vector<ScheduledModule>> &best_plan)
      -> std::queue<std::pair<std::vector<ScheduledModule>,
                              std::vector<std::shared_ptr<QueryNode>>>>;
  static auto GetLargestModulesSizes(
      const std::map<QueryOperationType, OperationPRModules> &hw_libary)
      -> std::unordered_map<QueryOperationType, int>;

  std::unique_ptr<PlanEvaluatorInterface> plan_evaluator_;
  std::unique_ptr<ElasticSchedulingGraphParser> scheduler_;
};
}  // namespace orkhestrafs::dbmstodspi