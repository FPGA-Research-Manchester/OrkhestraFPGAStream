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

#include "node_scheduler_interface.hpp"
#include "plan_evaluator_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to schedule nodes to groups of different FPGA runs using module
 * resource information.
 */
class ElasticResourceNodeScheduler : public NodeSchedulerInterface {
 public:
  ElasticResourceNodeScheduler(
      std::unique_ptr<PlanEvaluatorInterface> plan_evaluator)
      : plan_evaluator_{std::move(plan_evaluator)} {}

  auto FindAcceleratedQueryNodeSets(
      std::vector<std::shared_ptr<QueryNode>> starting_nodes,
      const std::map<ConfigurableModulesVector, std::string>
          &supported_accelerator_bitstreams,
      const std::map<QueryOperationType, std::vector<std::vector<int>>>
          &existing_modules_library,
      std::map<std::string,
               std::map<int, std::vector<std::pair<std::string, int>>>>
          &linked_nodes)
      -> std::queue<
          std::pair<ConfigurableModulesVector,
                    std::vector<std::shared_ptr<QueryNode>>>> override;

  auto GetNextSetOfRuns(
      std::vector<std::shared_ptr<QueryNode>>& available_nodes,
      const std::map<QueryOperationType, OperationPRModules> &hw_library,
      const std::vector<std::string> &first_node_names,
      std::vector<std::string> &starting_nodes,
      std::vector<std::string> &processed_nodes,
      std::map<std::string, SchedulingQueryNode> &graph,
      AcceleratorLibraryInterface &drivers,
      std::map<std::string, TableMetadata> &tables)
      -> std::queue<
          std::pair<ConfigurableModulesVector,
                    std::vector<std::shared_ptr<QueryNode>>>> override;

 private:
  static auto CalculateTimeLimit(
      const std::map<std::string, SchedulingQueryNode> &graph,
      const std::map<std::string, TableMetadata> &data_tables,
      double config_speed, double streaming_speed,
      const std::map<QueryOperationType, int> &operation_costs) -> double;
  static auto FindSharedPointerFromRootNodes(
      std::string searched_node_name, std::shared_ptr<QueryNode> current_node)
      -> std::shared_ptr<QueryNode>;
  static void RemoveUnnecessaryTables(
      const std::map<std::string, SchedulingQueryNode> &graph,
      std::map<std::string, TableMetadata> &tables);

  std::unique_ptr<PlanEvaluatorInterface> plan_evaluator_;
};
}  // namespace orkhestrafs::dbmstodspi