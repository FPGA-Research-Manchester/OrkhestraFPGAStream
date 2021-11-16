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
#include "query_scheduling_helper.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::table_data::SortedSequence;
using orkhestrafs::dbmstodspi::ElasticSchedulingGraphParser;
using orkhestrafs::dbmstodspi::PreSchedulingProcessor;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

void ElasticSchedulingGraphParser::PreprocessNodes(
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

auto ElasticSchedulingGraphParser::CurrentRunHasFirstModule(
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::vector<ScheduledModule>& current_run, std::string node_name,
    AcceleratorLibraryInterface& drivers) -> bool {
  for (const auto& scheduled_module : current_run) {
    if (scheduled_module.node_name != node_name &&
        drivers.IsNodeConstrainedToFirstInPipeline(
            scheduled_module.operation_type)) {
      return true;
    }
  }
  return false;
}

auto ElasticSchedulingGraphParser::RemoveUnavailableNodesInThisRun(
    const std::vector<std::string>& available_nodes,
    const std::vector<ScheduledModule>& current_run,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::map<std::string, SchedulingQueryNode>& graph,
    const std::vector<std::string>& constrained_first_nodes,
    const std::vector<std::string>& blocked_nodes,
    AcceleratorLibraryInterface& drivers) -> std::vector<std::string> {
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
    if (drivers.IsNodeConstrainedToFirstInPipeline(
            graph.at(node_name).operation) &&
        CurrentRunHasFirstModule(hw_library, current_run, node_name, drivers)) {
      resulting_nodes.erase(std::remove(resulting_nodes.begin(),
                                        resulting_nodes.end(), node_name),
                            resulting_nodes.end());
    }
    if (std::find(blocked_nodes.begin(), blocked_nodes.end(), node_name) !=
        blocked_nodes.end()) {
      resulting_nodes.erase(std::remove(resulting_nodes.begin(),
                                        resulting_nodes.end(), node_name),
                            resulting_nodes.end());
    }
  }
  return resulting_nodes;
}

auto ElasticSchedulingGraphParser::GetMinPositionInCurrentRun(
    const std::vector<ScheduledModule>& current_run, std::string node_name,
    const std::map<std::string, SchedulingQueryNode>& graph) -> int {
  std::vector<ScheduledModule> currently_scheduled_prereq_nodes;
  for (const auto& [previous_node_name, _] : graph.at(node_name).before_nodes) {
    for (const auto& module_placement : current_run) {
      if (previous_node_name == module_placement.node_name) {
        currently_scheduled_prereq_nodes.push_back(module_placement);
      }
    }
  }
  if (!currently_scheduled_prereq_nodes.empty()) {
    int current_min = 0;
    for (const auto& module_placement : currently_scheduled_prereq_nodes) {
      if (module_placement.position.second > current_min) {
        current_min = module_placement.position.second;
      }
    }
    // Assuming current min isn't 0 or negative somehow.
    return current_min + 1;
  } else {
    return 0;
  }
}

auto ElasticSchedulingGraphParser::GetTakenColumns(
    const std::vector<ScheduledModule>& current_run)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> taken_columns;
  for (const auto& module_placement : current_run) {
    taken_columns.push_back(module_placement.position);
  }
  return taken_columns;
}

auto ElasticSchedulingGraphParser::GetModuleIndex(
    int start_location_index,
    const std::vector<std::pair<int, int>>& taken_positions) -> int {
  if (taken_positions.empty()) {
    throw std::runtime_error("Taken positions can't be empty!");
  }
  if (start_location_index < taken_positions.at(0).first) {
    return 0;
  }
  for (int module_index = taken_positions.size() - 1; module_index >= 0;
       module_index--) {
    if (start_location_index > taken_positions.at(module_index).second) {
      return module_index + 1;
    }
  }
  return taken_positions.size();
}

auto ElasticSchedulingGraphParser::FindAllAvailableBitstreamsAfterMinPos(
    QueryOperationType current_operation, int min_position,
    const std::vector<std::pair<int, int>>& taken_positions,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::vector<std::vector<std::string>>& bitstream_start_locations)
    -> std::vector<std::tuple<int, int, int>> {
  std::vector<std::tuple<int, int, int>> all_positions_and_bitstream_indexes;
  for (int start_location_index = min_position;
       start_location_index < bitstream_start_locations.size();
       start_location_index++) {
    for (const auto& bitstream_name :
         bitstream_start_locations.at(start_location_index)) {
      auto bitstream_index =
          std::find(hw_library.at(current_operation)
                        .starting_locations.at(start_location_index)
                        .begin(),
                    hw_library.at(current_operation)
                        .starting_locations.at(start_location_index)
                        .end(),
                    bitstream_name) -
          hw_library.at(current_operation)
              .starting_locations.at(start_location_index)
              .begin();
      if (taken_positions.empty()) {
        all_positions_and_bitstream_indexes.push_back(
            {0, start_location_index, bitstream_index});
      } else {
        auto module_index =
            GetModuleIndex(start_location_index, taken_positions);
        auto end_index = start_location_index +
                         hw_library.at(current_operation)
                             .bitstream_map.at(bitstream_name)
                             .length -
                         1;
        if ((taken_positions.size() == module_index &&
             taken_positions.at(module_index - 1).second <
                 start_location_index) ||
            (taken_positions.size() != module_index &&
             taken_positions.at(module_index).first > end_index)) {
          all_positions_and_bitstream_indexes.push_back(
              {module_index, start_location_index, bitstream_index});
        }
      }
    }
  }
  return all_positions_and_bitstream_indexes;
}

auto ElasticSchedulingGraphParser::GetBitstreamEndFromLibrary(
    int chosen_bitstream_index, int chosen_column_position,
    QueryOperationType current_operation,
    const std::map<QueryOperationType, OperationPRModules>& hw_library)
    -> std::pair<std::string, int> {
  auto chosen_bitstream_name =
      hw_library.at(current_operation)
          .starting_locations.at(chosen_column_position)
          .at(chosen_bitstream_index);
  auto end_index = chosen_column_position +
                   hw_library.at(current_operation)
                       .bitstream_map.at(chosen_bitstream_name)
                       .length -
                   1;
  return {chosen_bitstream_name, end_index};
}

void ElasticSchedulingGraphParser::ReduceSelectionAccordingToHeuristics(
    std::vector<std::pair<int, ScheduledModule>>& resulting_module_placements,
    const std::vector<std::vector<ModuleSelection>>& heuristics) {
  std::vector<std::vector<std::pair<int, ScheduledModule>>>
      all_selected_placements;
  for (const auto& module_placement_clause : heuristics) {
    auto current_selected_placements = resulting_module_placements;
    for (const auto& placement_selection_function : module_placement_clause) {
      current_selected_placements =
          placement_selection_function.SelectAccordingToMode(
              current_selected_placements);
    }
    all_selected_placements.push_back(current_selected_placements);
  }
  resulting_module_placements.clear();
  for (const auto& chosen_module_set : all_selected_placements) {
    for (const auto& chosen_module : chosen_module_set) {
      if (std::find(resulting_module_placements.begin(),
                    resulting_module_placements.end(),
                    chosen_module) == resulting_module_placements.end()) {
        resulting_module_placements.push_back(chosen_module);
      }
    }
  }
}

auto ElasticSchedulingGraphParser::GetChosenModulePlacements(
    std::string node_name,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    std::pair<int, int>& statistics_counters,
    QueryOperationType current_operation,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    int min_position, const std::vector<std::pair<int, int>>& taken_positions,
    const std::vector<std::vector<std::string>>& bitstream_start_locations)
    -> std::vector<std::pair<int, ScheduledModule>> {
  auto available_bitstreams = FindAllAvailableBitstreamsAfterMinPos(
      current_operation, min_position, taken_positions, hw_library,
      bitstream_start_locations);
  std::vector<std::pair<int, ScheduledModule>> resulting_module_placements;
  if (!available_bitstreams.empty()) {
    for (const auto& [chosen_module_position, chosen_column_position,
                      chosen_bitstream_index] : available_bitstreams) {
      auto [chosen_bitstream, end_index] = GetBitstreamEndFromLibrary(
          chosen_bitstream_index, chosen_column_position, current_operation,
          hw_library);
      resulting_module_placements.push_back(
          {chosen_module_position,
           {node_name,
            current_operation,
            chosen_bitstream,
            {chosen_column_position, end_index}}});
    }
    statistics_counters.first += resulting_module_placements.size();
    ReduceSelectionAccordingToHeuristics(resulting_module_placements,
                                         heuristics);
    statistics_counters.first -= resulting_module_placements.size();
    statistics_counters.second += 1;
  }
  return resulting_module_placements;
}

auto ElasticSchedulingGraphParser::GetScheduledModulesForNodeAfterPos(
    const std::map<std::string, SchedulingQueryNode>& graph, int min_position,
    std::string node_name,
    const std::vector<std::pair<int, int>>& taken_positions,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::pair<std::vector<std::vector<ModuleSelection>>,
                    std::vector<std::vector<ModuleSelection>>>& heuristics,
    std::pair<int, int>& statistics_counters)
    -> std::vector<std::pair<int, ScheduledModule>> {
  auto current_operation = graph.at(node_name).operation;
  std::vector<std::pair<int, ScheduledModule>> available_module_placements;
  // Will never be empty but just in case
  if (!graph.at(node_name).satisfying_bitstreams.empty()) {
    available_module_placements = GetChosenModulePlacements(
        node_name, hw_library, statistics_counters, current_operation,
        heuristics.first, min_position, taken_positions,
        graph.at(node_name).satisfying_bitstreams);
  }
  if (available_module_placements.empty()) {
    available_module_placements = GetChosenModulePlacements(
        node_name, hw_library, statistics_counters, current_operation,
        heuristics.second, min_position, taken_positions,
        hw_library.at(graph.at(node_name).operation).starting_locations);
  }
  return available_module_placements;
}

auto ElasticSchedulingGraphParser::CheckForSkippableSortOperations(
    const std::map<std::string, SchedulingQueryNode>& new_graph,
    const std::map<std::string, TableMetadata>& new_tables,
    std::string node_name,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    AcceleratorLibraryInterface& drivers) -> std::vector<std::string> {
  std::vector<std::string> skipped_nodes;
  auto all_tables_sorted = !std::any_of(
      new_graph.at(node_name).data_tables.begin(),
      new_graph.at(node_name).data_tables.end(), [&](std::string table_name) {
        return !QuerySchedulingHelper::IsTableSorted(new_tables.at(table_name));
      });
  if (all_tables_sorted) {
    for (const auto& current_next_node_name :
         new_graph.at(node_name).after_nodes) {
      if (drivers.IsOperationSorting(
              new_graph.at(current_next_node_name).operation)) {
        skipped_nodes.push_back(current_next_node_name);
      }
    }
  }
  return skipped_nodes;
}

auto ElasticSchedulingGraphParser::FindMissingUtility(
    const std::vector<int>& bitstream_capacity,
    std::vector<int>& missing_capacity, const std::vector<int>& node_capacity)
    -> bool {
  bool is_node_fully_processed = true;
  if (bitstream_capacity.size() != node_capacity.size()) {
    throw std::runtime_error("Capacity parameters don't match!");
  }
  for (int capacity_parameter_index = 0;
       capacity_parameter_index < bitstream_capacity.size();
       capacity_parameter_index++) {
    missing_capacity.push_back(node_capacity.at(capacity_parameter_index) -
                               bitstream_capacity.at(capacity_parameter_index));
    if (missing_capacity.at(capacity_parameter_index) > 0) {
      is_node_fully_processed = false;
    }
  }
  return is_node_fully_processed;
}

void ElasticSchedulingGraphParser::UpdateGraphCapacities(
    const std::map<std::string, SchedulingQueryNode>& graph,
    const std::vector<int>& missing_utility,
    std::map<std::string, SchedulingQueryNode>& new_graph,
    std::string node_name, bool is_node_fully_processed) {
  if (!is_node_fully_processed) {
    new_graph.at(node_name) = graph.at(node_name);
    std::vector<int> new_capacity_values;
    for (int capacity_parameter_index = 0;
         capacity_parameter_index < missing_utility.size();
         capacity_parameter_index++) {
      new_capacity_values.push_back(
          std::max(0, missing_utility.at(capacity_parameter_index)));
    }
    new_graph.at(node_name).capacity = new_capacity_values;
  }
}

auto ElasticSchedulingGraphParser::GetResultingTables(
    const std::vector<std::string>& table_names,
    AcceleratorLibraryInterface& drivers,
    const std::map<std::string, TableMetadata>& tables,
    QueryOperationType operation) -> std::vector<std::string> {
  if (drivers.IsInputSupposedToBeSorted(operation)) {
    for (const auto& table_name : table_names) {
      if (!QuerySchedulingHelper::IsTableSorted(tables.at(table_name))) {
        throw std::runtime_error("Table should be sorted!");
      }
    }
  }
  return drivers.GetResultingTables(operation, table_names, tables);
}

void ElasticSchedulingGraphParser::UpdateNextNodeTables(
    const std::map<std::string, SchedulingQueryNode>& graph,
    std::string node_name,
    std::map<std::string, SchedulingQueryNode>& new_graph,
    std::vector<std::string> skipped_nodes,
    const std::vector<std::string> resulting_tables) {
  QuerySchedulingHelper::AddNewTableToNextNodes(new_graph, node_name,
                                                resulting_tables);
  new_graph.erase(node_name);
  for (const auto& skipped_node : skipped_nodes) {
    QuerySchedulingHelper::AddNewTableToNextNodes(new_graph, skipped_node,
                                                  resulting_tables);
    new_graph.erase(skipped_node);
  }
}

auto ElasticSchedulingGraphParser::UpdateGraph(
    const std::map<std::string, SchedulingQueryNode>& graph,
    std::string bitstream,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    std::string node_name, const std::vector<int>& capacity,
    QueryOperationType operation, AcceleratorLibraryInterface& drivers)
    -> std::tuple<std::map<std::string, SchedulingQueryNode>,
                  std::map<std::string, TableMetadata>, bool,
                  std::vector<std::string>> {
  auto new_graph = graph;
  auto new_tables = data_tables;
  std::vector<std::string> skipped_nodes;
  bool is_node_fully_processed = false;
  if (drivers.IsOperationSorting(operation)) {
    is_node_fully_processed = drivers.UpdateDataTable(
        operation,
        hw_library.at(operation).bitstream_map.at(bitstream).capacity,
        graph.at(node_name).data_tables, data_tables, new_tables);
    skipped_nodes = CheckForSkippableSortOperations(
        new_graph, new_tables, node_name, hw_library, drivers);
  } else {
    std::vector<int> missing_utility;
    is_node_fully_processed = FindMissingUtility(
        hw_library.at(operation).bitstream_map.at(bitstream).capacity,
        missing_utility, capacity);
    UpdateGraphCapacities(graph, missing_utility, new_graph, node_name,
                          is_node_fully_processed);
  }
  if (is_node_fully_processed) {
    auto resulting_table = GetResultingTables(graph.at(node_name).data_tables,
                                              drivers, new_tables, operation);
    UpdateNextNodeTables(graph, node_name, new_graph, skipped_nodes,
                         resulting_table);
  }
  return {new_graph, new_tables, is_node_fully_processed, skipped_nodes};
}

auto ElasticSchedulingGraphParser::CreateNewAvailableNodes(
    const std::map<std::string, SchedulingQueryNode>& graph,
    const std::vector<std::string>& available_nodes,
    const std::vector<std::string>& processed_nodes, std::string node_name,
    bool satisfied_requirements)
    -> std::pair<std::vector<std::string>, std::vector<std::string>> {
  auto new_available_nodes = available_nodes;
  auto new_processed_nodes = processed_nodes;
  if (satisfied_requirements) {
    new_available_nodes.erase(std::remove(new_available_nodes.begin(),
                                          new_available_nodes.end(), node_name),
                              new_available_nodes.end());
    new_processed_nodes.push_back(node_name);
    auto next_nodes =
        QuerySchedulingHelper::GetNewAvailableNodesAfterSchedulingGivenNode(
            node_name, new_processed_nodes, graph);
    new_available_nodes.insert(new_available_nodes.end(), next_nodes.begin(),
                               next_nodes.end());
  }
  return {new_available_nodes, new_processed_nodes};
}

auto ElasticSchedulingGraphParser::IsTableEqualForGivenNode(
    const std::map<std::string, SchedulingQueryNode>& graph,
    const std::map<std::string, SchedulingQueryNode>& new_graph,
    std::string node_name,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::map<std::string, TableMetadata>& new_tables) -> bool {
  for (int table_index = 0;
       table_index < graph.at(node_name).data_tables.size(); table_index++) {
    auto old_table_data =
        data_tables.at(graph.at(node_name).data_tables.at(table_index));
    auto new_table_data =
        new_tables.at(new_graph.at(node_name).data_tables.at(table_index));
    if (old_table_data.record_count != new_table_data.record_count &&
        old_table_data.record_size != new_table_data.record_size &&
        old_table_data.sorted_status.size() !=
            new_table_data.sorted_status.size() &&
        !std::equal(old_table_data.sorted_status.begin(),
                    old_table_data.sorted_status.end(),
                    new_table_data.sorted_status.begin(),
                    [](const SortedSequence& l, const SortedSequence& r) {
                      return l.length == r.length &&
                             l.start_position == r.start_position;
                    })) {
      return true;
    }
  }
  return false;
}

void ElasticSchedulingGraphParser::UpdateSatisfyingBitstreamsList(
    std::string node_name,
    const std::map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, SchedulingQueryNode>& new_graph,
    const std::vector<std::string>& new_available_nodes,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const std::map<std::string, TableMetadata>& data_tables,
    std::map<std::string, TableMetadata>& new_tables,
    const std::vector<std::string>& new_processed_nodes,
    AcceleratorLibraryInterface& drivers) {
  if (std::any_of(
          new_graph.begin(), new_graph.end(), [&](const auto& map_entry) {
            return graph.at(map_entry.first).capacity !=
                       new_graph.at(map_entry.first).capacity ||
                   !IsTableEqualForGivenNode(graph, new_graph, map_entry.first,
                                             data_tables, new_tables);
          })) {
    PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
        hw_library, new_graph, new_tables, drivers, new_available_nodes,
        new_processed_nodes);
  } else {
    if (std::any_of(graph.at(node_name).after_nodes.begin(),
                    graph.at(node_name).after_nodes.end(),
                    [&](std::string next_node_name) {
                      return !IsTableEqualForGivenNode(graph, new_graph,
                                                       next_node_name,
                                                       data_tables, new_tables);
                    })) {
      PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
          hw_library, new_graph, new_tables, drivers, new_available_nodes,
          new_processed_nodes);
    }
  }
}

auto ElasticSchedulingGraphParser::GetNewBlockedNodes(
    const std::vector<std::string>& next_run_blocked_nodes,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    const ScheduledModule& module_placement,
    const std::map<std::string, SchedulingQueryNode>& graph,
    AcceleratorLibraryInterface& drivers) -> std::vector<std::string> {
  auto new_next_run_blocked_nodes = next_run_blocked_nodes;
  if (std::find(next_run_blocked_nodes.begin(), next_run_blocked_nodes.end(),
                module_placement.node_name) == next_run_blocked_nodes.end()) {
    if (drivers.IsOperationReducingData(module_placement.operation_type)) {
      new_next_run_blocked_nodes.push_back(module_placement.node_name);
      for (const auto& next_node_name :
           graph.at(module_placement.node_name).after_nodes) {
        if (std::find(next_run_blocked_nodes.begin(),
                      next_run_blocked_nodes.end(),
                      next_node_name) == next_run_blocked_nodes.end()) {
          new_next_run_blocked_nodes.push_back(next_node_name);
        }
      }
    }
  } else {
    for (const auto& next_node_name :
         graph.at(module_placement.node_name).after_nodes) {
      if (std::find(next_run_blocked_nodes.begin(),
                    next_run_blocked_nodes.end(),
                    next_node_name) == next_run_blocked_nodes.end()) {
        new_next_run_blocked_nodes.push_back(next_node_name);
      }
    }
  }
  return new_next_run_blocked_nodes;
}

void ElasticSchedulingGraphParser::FindNextModulePlacement(
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
    const std::pair<std::vector<std::vector<ModuleSelection>>,
                    std::vector<std::vector<ModuleSelection>>>& heuristics,
    std::pair<int, int>& statistics_counters,
    const std::vector<std::string>& constrained_first_nodes,
    std::vector<std::string> blocked_nodes,
    std::vector<std::string> next_run_blocked_nodes,
    AcceleratorLibraryInterface& drivers) {
  auto [new_graph, new_tables, satisfied_requirements, skipped_node_names] =
      UpdateGraph(graph, module_placement.bitstream, data_tables, hw_library,
                  node_name, graph.at(node_name).capacity,
                  graph.at(node_name).operation, drivers);

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
                                 new_tables, new_processed_nodes, drivers);

  auto new_next_run_blocked = GetNewBlockedNodes(
      next_run_blocked_nodes, hw_library, module_placement, graph, drivers);

  PlaceNodesRecursively(
      new_available_nodes, new_processed_nodes, new_graph, current_run,
      current_plan, resulting_plan, reduce_single_runs, hw_library, min_runs,
      new_tables, heuristics, statistics_counters, constrained_first_nodes,
      blocked_nodes, new_next_run_blocked, drivers);
}

// TODO: The constants could be changed to be class variables.
void ElasticSchedulingGraphParser::PlaceNodesRecursively(
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
    const std::pair<std::vector<std::vector<ModuleSelection>>,
                    std::vector<std::vector<ModuleSelection>>>& heuristics,
    std::pair<int, int>& statistics_counters,
    const std::vector<std::string>& constrained_first_nodes,
    std::vector<std::string> blocked_nodes,
    std::vector<std::string> next_run_blocked_nodes,
    AcceleratorLibraryInterface& drivers) {
  if (current_plan.size() <= min_runs) {
    std::sort(blocked_nodes.begin(), blocked_nodes.end());
    std::sort(available_nodes.begin(), available_nodes.end());
    if (!available_nodes.empty() &&
        !std::includes(blocked_nodes.begin(), blocked_nodes.end(),
                       available_nodes.begin(), available_nodes.end())) {
      auto available_nodes_in_this_run = RemoveUnavailableNodesInThisRun(
          available_nodes, current_run, hw_library, graph,
          constrained_first_nodes, blocked_nodes, drivers);
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
                  blocked_nodes, next_run_blocked_nodes, drivers);
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
                constrained_first_nodes, new_blocked_nodes, {}, drivers);
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
                  new_blocked_nodes, {}, drivers);
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
