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

#include <map>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "accelerator_library_interface.hpp"
#include "config.hpp"
#include "operation_types.hpp"
#include "pr_module_data.hpp"
#include "query_scheduling_data.hpp"
#include "scheduled_module.hpp"
#include "scheduling_data.hpp"
#include "scheduling_query_node.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

using orkhestrafs::core_interfaces::Config;
using orkhestrafs::core_interfaces::table_data::TableMetadata;
using orkhestrafs::dbmstodspi::AcceleratorLibraryInterface;
using orkhestrafs::dbmstodspi::ScheduledModule;
using orkhestrafs::dbmstodspi::SchedulingQueryNode;
using orkhestrafs::dbmstodspi::scheduling_data::ExecutionPlanSchedulingData;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface for schedulers
 */
class NodeSchedulerInterface {
 public:
  virtual ~NodeSchedulerInterface() = default;

  /**
   * @brief Find groups of accelerated query nodes which can be run in the
   * FPGA with multiple runs.
   * @param query_nodes Pointers of all of the leaf nodes
   * @param first_node_names Names of the leaf nodes.
   * @param starting_nodes Input vector of leaf nodes from which the parsing can
   * begin.
   * @param processed_nodes Nodes that have been executed already.
   * @param graph Graph of all of the nodes that haven't been executed already.
   * @param drivers Drivers of the operators
   * @param tables Table metadata.
   * @param current_configuration Currently loaded modules.
   * @param config All scheduling related metadata including HW library
   * @return Queue of groups of accelerated query
   * nodes to be accelerated next.
   */
  virtual auto GetNextSetOfRuns(
      std::vector<std::shared_ptr<QueryNode>>& query_nodes,
      const std::unordered_set<std::string>& first_node_names,
      std::unordered_set<std::string>& starting_nodes,
      std::unordered_set<std::string>& processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode>& graph,
      AcceleratorLibraryInterface& drivers,
      std::map<std::string, TableMetadata>& tables,
      const std::vector<ScheduledModule>& current_configuration,
      const Config& config)
      -> std::queue<std::pair<std::vector<ScheduledModule>,
                              std::vector<std::shared_ptr<QueryNode>>>> = 0;

  // Core scheduling method for benchmarking
  virtual auto ScheduleAndGetAllPlans(
      const std::unordered_set<std::string>& first_node_names,
      std::unordered_set<std::string>& starting_nodes,
      std::unordered_set<std::string>& processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode>& graph,
      AcceleratorLibraryInterface& drivers,
      std::map<std::string, TableMetadata>& tables, const Config& config)
      -> std::tuple<int,
                    std::map<std::vector<std::vector<ScheduledModule>>,
                             ExecutionPlanSchedulingData>,
                    long long, bool, std::pair<int, int>> = 0;

  virtual void BenchmarkScheduling(
      const std::unordered_set<std::string>& first_node_names,
      std::unordered_set<std::string>& starting_nodes,
      std::unordered_set<std::string>& processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode>& graph,
      AcceleratorLibraryInterface& drivers,
      std::map<std::string, TableMetadata>& tables,
      std::vector<ScheduledModule>& current_configuration, const Config& config,
      std::map<std::string, double>& benchmark_data) = 0;
};

}  // namespace orkhestrafs::dbmstodspi