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

#include "elastic_scheduling_graph_parser.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "pre_scheduling_processor.hpp"

using orkhestrafs::dbmstodspi::ElastiSchedulingGraphParser;
using orkhestrafs::dbmstodspi::PreSchedulingProcessor;

void ElastiSchedulingGraphParser::PreprocessNodes(
    const std::vector<std::string>& available_nodes,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::vector<std::string>& processed_nodes,
    std::map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& data_tables,
    AcceleratorLibraryInterface& accelerator_library) {
  PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
      hw_library, graph, data_tables, accelerator_library, available_nodes,
      processed_nodes);
}

auto ElastiSchedulingGraphParser::RemoveUnavailableNodesInThisRun(
    const std::vector<std::string>& available_nodes,
    const std::vector<ScheduledModule>& current_run,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::map<std::string, SchedulingQueryNode>& graph,
    const std::vector<std::string>& constrained_first_nodes,
    const std::vector<std::string>& blocked_nodes) -> std::vector<std::string> {
  auto resulting_nodes = available_nodes;
  for (const auto& node_name : available_nodes) {
    if (std::find(constrained_first_nodes.begin(),
                  constrained_first_nodes.end(),
                  node_name) != constrained_first_nodes.end()) {
      for (const auto& module : current_run) {
        for (const auto& [before_node_name, _] :
             graph.at(node_name).before_nodes) {
          if (module.node_name == before_node_name) {
            resulting_nodes.erase(std::remove(resulting_nodes.begin(),
                                              resulting_nodes.end(), node_name),
                                  resulting_nodes.end());
          }
        }
      }
    }
    auto operation = graph.at(node_name).operation;
    // TODO: Check for first modules
    if (std::find(blocked_nodes.begin(), blocked_nodes.end(), node_name) !=
        blocked_nodes.end()) {
      resulting_nodes.erase(std::remove(resulting_nodes.begin(),
                                        resulting_nodes.end(), node_name),
                            resulting_nodes.end());
    }
  }
  return resulting_nodes;
}

auto ElastiSchedulingGraphParser::GetMinPositionInCurrentRun(
    const std::vector<ScheduledModule>& current_run, std::string node_name,
    const std::map<std::string, SchedulingQueryNode>& graph) -> int {
  return 0;
}

auto ElastiSchedulingGraphParser::GetTakenColumns(
    const std::vector<ScheduledModule>& current_run)
    -> std::vector<std::pair<int, int>> {
  return {};
}

auto ElastiSchedulingGraphParser::GetScheduledModulesForNodeAfterPos(
    const std::map<std::string, SchedulingQueryNode>& graph, int min_position,
    std::string node_name,
    const std::vector<std::pair<int, int>>& taken_positions,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    std::pair<int, int>& statistics_counters)
    -> std::vector<std::pair<int, ScheduledModule>> {
  return {};
}

// TODO: check if you can make some inputs const and which ones references!
auto ElastiSchedulingGraphParser::UpdateGraph(
    std::map<std::string, SchedulingQueryNode> graph, std::string bitstream,
    std::map<std::string, TableMetadata> data_tables,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    std::string node_name, std::vector<int> capacity,
    QueryOperationType operation)
    -> std::tuple<std::map<std::string, SchedulingQueryNode>,
                  std::map<std::string, TableMetadata>, bool,
                  std::vector<std::string>> {
  return {};
}

auto ElastiSchedulingGraphParser::CreateNewAvailableNodes(
    std::map<std::string, SchedulingQueryNode> graph,
    std::vector<std::string> available_nodes,
    std::vector<std::string> processed_nodes, std::string node_name,
    bool satisfied_requirements)
    -> std::pair<std::vector<std::string>, std::vector<std::string>> {
  return {};
}

void ElastiSchedulingGraphParser::UpdateSatisfyingBitstreamsList(
    std::string node_name, std::map<std::string, SchedulingQueryNode> graph,
    std::map<std::string, SchedulingQueryNode> new_graph,
    std::vector<std::string> new_available_nodes,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    std::map<std::string, TableMetadata> data_tables,
    std::map<std::string, TableMetadata> new_tables,
    std::vector<std::string> new_processed_nodes) {}

auto ElastiSchedulingGraphParser::GetNewBlockedNodes(
    std::vector<std::string> next_run_blocked_nodes,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const ScheduledModule& module_placement,
    std::map<std::string, SchedulingQueryNode> graph)
    -> std::vector<std::string> {
  return {};
}

void ElastiSchedulingGraphParser::FindNextModulePlacement(
    std::map<std::string, SchedulingQueryNode> graph,
    std::map<std::vector<std::vector<ScheduledModule>>,
             ExecutionPlanSchedulingData>& resulting_plan,
    std::vector<std::string> available_nodes,
    std::vector<std::vector<ScheduledModule>> current_plan,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const ScheduledModule& module_placement,
    std::vector<ScheduledModule> current_run, std::string node_name,
    std::vector<std::string> processed_nodes, bool reduce_single_runs,
    int& min_runs, std::map<std::string, TableMetadata> data_tables,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    std::pair<int, int>& statistics_counters,
    const std::vector<std::string>& constrained_first_nodes,
    std::vector<std::string> blocked_nodes,
    std::vector<std::string> next_run_blocked_nodes) {
  auto [new_graph, new_tables, satisfied_requirements, skipped_node_names] =
      UpdateGraph(graph, module_placement.bitstream, data_tables, hw_library,
                  node_name, graph.at(node_name).capacity,
                  graph.at(node_name).operation);

  auto [new_available_nodes, new_processed_nodes] =
      CreateNewAvailableNodes(graph, available_nodes, processed_nodes,
                              node_name, satisfied_requirements);

  for (const auto& skipped_node_name : skipped_node_names) {
    if (std::find(new_available_nodes.begin(), new_available_nodes.end(),
                  skipped_node_name) == new_available_nodes.end()) {
      throw std::runtime_error("Skipped nodes marked in the wrong order!");
    } else {
      auto new_nodes_pair = CreateNewAvailableNodes(
          graph, new_available_nodes, new_processed_nodes, skipped_node_name,
          satisfied_requirements);
      new_available_nodes = new_nodes_pair.first;
      new_processed_nodes = new_nodes_pair.second;
    }
  }

  UpdateSatisfyingBitstreamsList(node_name, graph, new_graph,
                                 new_available_nodes, hw_library, data_tables,
                                 new_tables, new_processed_nodes);

  auto new_next_run_blocked = GetNewBlockedNodes(
      next_run_blocked_nodes, hw_library, module_placement, graph);

  PlaceNodesRecursively(
      new_available_nodes, new_processed_nodes, new_graph, current_run,
      current_plan, resulting_plan, reduce_single_runs, hw_library, min_runs,
      new_tables, heuristics, statistics_counters, constrained_first_nodes,
      blocked_nodes, next_run_blocked_nodes);
}

// TODO: The constants could be changed to be class variables.
void ElastiSchedulingGraphParser::PlaceNodesRecursively(
    std::vector<std::string> available_nodes,
    std::vector<std::string> processed_nodes,
    std::map<std::string, SchedulingQueryNode> graph,
    std::vector<ScheduledModule> current_run,
    std::vector<std::vector<ScheduledModule>> current_plan,
    std::map<std::vector<std::vector<ScheduledModule>>,
             ExecutionPlanSchedulingData>& resulting_plan,
    bool reduce_single_runs,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    int& min_runs, std::map<std::string, TableMetadata> data_tables,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    std::pair<int, int>& statistics_counters,
    const std::vector<std::string>& constrained_first_nodes,
    std::vector<std::string> blocked_nodes,
    std::vector<std::string> next_run_blocked_nodes) {
  if (current_plan.size() <= min_runs) {
    if (!available_nodes.empty() &&
        !std::includes(available_nodes.begin(), available_nodes.end(),
                       blocked_nodes.begin(), blocked_nodes.end())) {
      auto available_nodes_in_this_run = RemoveUnavailableNodesInThisRun(
          available_nodes, current_run, hw_library, graph,
          constrained_first_nodes, blocked_nodes);
      for (const auto& node_name : available_nodes) {
        std::vector<std::pair<int, ScheduledModule>>
            available_module_placements;
        if (std::find(available_nodes_in_this_run.begin(),
                      available_nodes_in_this_run.end(),
                      node_name) != available_nodes_in_this_run.end()) {
          auto min_position =
              GetMinPositionInCurrentRun(current_run, node_name, graph);
          auto taken_positions = GetTakenColumns(current_run);
          available_module_placements = GetScheduledModulesForNodeAfterPos(
              graph, min_position, node_name, taken_positions, hw_library,
              heuristics, statistics_counters);
          if (!available_module_placements.empty()) {
            for (const auto& [module_index, module_placement] :
                 available_module_placements) {
              auto new_current_run = current_run;
              new_current_run.insert(new_current_run.begin() + module_index,
                                     module_placement);
              FindNextModulePlacement(
                  graph, resulting_plan, available_nodes, current_plan,
                  hw_library, module_placement, new_current_run, node_name,
                  processed_nodes, reduce_single_runs, min_runs, data_tables,
                  heuristics, statistics_counters, constrained_first_nodes,
                  blocked_nodes, next_run_blocked_nodes);
            }
          }
        }
        if ((available_module_placements.empty() || !reduce_single_runs) &&
            std::find(blocked_nodes.begin(), blocked_nodes.end(), node_name) ==
                blocked_nodes.end()) {
          auto new_blocked_nodes = blocked_nodes;
          new_blocked_nodes.insert(new_blocked_nodes.end(),
                                   next_run_blocked_nodes.begin(),
                                   next_run_blocked_nodes.end());
          if (std::find(new_blocked_nodes.begin(), new_blocked_nodes.end(),
                        node_name) != new_blocked_nodes.end()) {
            PlaceNodesRecursively(
                available_nodes, processed_nodes, graph, current_run,
                current_plan, resulting_plan, reduce_single_runs, hw_library,
                min_runs, data_tables, heuristics, statistics_counters,
                constrained_first_nodes, new_blocked_nodes, {});
            return;
          }

          auto new_current_plan = current_plan;
          if (!current_run.empty()) {
            new_current_plan.push_back(current_run);
          }
          available_module_placements = GetScheduledModulesForNodeAfterPos(
              graph, 0, node_name, {}, hw_library, heuristics,
              statistics_counters);
          if (!available_module_placements.empty()) {
            for (const auto& [_, module_placement] :
                 available_module_placements) {
              FindNextModulePlacement(
                  graph, resulting_plan, available_nodes, new_current_plan,
                  hw_library, module_placement, {module_placement}, node_name,
                  processed_nodes, reduce_single_runs, min_runs, data_tables,
                  heuristics, statistics_counters, constrained_first_nodes,
                  new_blocked_nodes, {});
            }
          } else {
            throw std::runtime_error(
                "Should be able to place nodes in an empty run!");
          }
        }
      }
    } else {
      current_plan.push_back(current_run);
      ExecutionPlanSchedulingData current_scheduling_data = {
          processed_nodes, available_nodes, graph, data_tables};
      if (const auto& [it, inserted] =
              resulting_plan.try_emplace(current_plan, current_scheduling_data);
          inserted) {
        if (current_plan.size() < min_runs) {
          min_runs = current_plan.size();
        }
      }
    }
  }
}
