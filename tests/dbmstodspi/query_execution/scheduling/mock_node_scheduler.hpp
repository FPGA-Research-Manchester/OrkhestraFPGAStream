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

#include "gmock/gmock.h"
#include "node_scheduler_interface.hpp"

using orkhestrafs::dbmstodspi::NodeSchedulerInterface;

class MockNodeScheduler : public NodeSchedulerInterface {
 private:
  using NodeVector = std::vector<QueryNode*>;
  using ResultingPlanQueue =
      std::queue<std::pair<std::vector<ScheduledModule>, NodeVector>>;
  using AcceleratorMap = std::map<ConfigurableModulesVector, std::string>;
  using LinkedNodesMap =
      std::map<std::string,
               std::map<int, std::vector<std::pair<std::string, int>>>>;
  using ModulesMap =
      std::map<QueryOperationType, std::vector<std::vector<int>>>;
  using HWLibraryMap = std::map<QueryOperationType, OperationPRModules>;
  using SchedulingNodeMap =
      std::unordered_map<std::string, SchedulingQueryNode>;
  using TableMap = std::map<std::string, TableMetadata>;
  using AllPlans =
      std::tuple<int,
                 std::map<std::vector<std::vector<ScheduledModule>>,
                          ExecutionPlanSchedulingData>,
                 long long, bool, std::pair<int, int>>;
  using BenchmarkMap = std::map<std::string, double>;
  using Counter = std::unordered_map<std::string, int>;
  using ConfigurationVector = std::vector<ScheduledModule>;

 public:
  MOCK_METHOD(long, GetTime, (), (override));
  MOCK_METHOD(
      ResultingPlanQueue, GetNextSetOfRuns,
      (std::vector<QueryNode*> & query_nodes,
       const std::unordered_set<std::string>& first_node_names,
       std::unordered_set<std::string> starting_nodes, SchedulingNodeMap graph,
       AcceleratorLibraryInterface& drivers, TableMap& tables,
       const ConfigurationVector& current_configuration, const Config& config,
       std::unordered_set<std::string>& skipped_nodes, Counter& table_counter,
       const std::unordered_set<std::string>& blocked_nodes),
      (override));

  MOCK_METHOD(AllPlans, ScheduleAndGetAllPlans,
              (const std::unordered_set<std::string>& starting_nodes,
               const std::unordered_set<std::string>& processed_nodes,
               const SchedulingNodeMap& graph, const TableMap& tables,
               const Config& config,
               const std::unordered_set<std::string>& blocked_nodes),
              (override));

  MOCK_METHOD(void, BenchmarkScheduling,
              (const std::unordered_set<std::string>& first_node_names,
               std::unordered_set<std::string> starting_nodes,
               std::unordered_set<std::string>& processed_nodes,
               SchedulingNodeMap graph, AcceleratorLibraryInterface& drivers,
               TableMap& tables, ConfigurationVector& current_configuration,
               const Config& config, BenchmarkMap& benchmark_data,
               const std::unordered_set<std::string>& blocked_nodes),
              (override));
};