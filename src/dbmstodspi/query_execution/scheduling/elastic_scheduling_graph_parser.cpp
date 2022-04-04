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
#include "time_limit_execption.hpp"
#include "util.hpp"

using orkhestrafs::core_interfaces::table_data::SortedSequence;
using orkhestrafs::dbmstodspi::ElasticSchedulingGraphParser;
using orkhestrafs::dbmstodspi::PairHash;
using orkhestrafs::dbmstodspi::PreSchedulingProcessor;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;
using orkhestrafs::dbmstodspi::TimeLimitException;

void ElasticSchedulingGraphParser::PreprocessNodes(
    std::unordered_set<std::string>& available_nodes,
    const std::unordered_set<std::string>& processed_nodes,
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& data_tables,
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    AcceleratorLibraryInterface& drivers) {
  PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
      hw_library, graph, data_tables, drivers, available_nodes,
      processed_nodes);
}

auto ElasticSchedulingGraphParser::CurrentRunHasFirstModule(
    const std::vector<ScheduledModule>& current_run,
    const std::string& node_name) -> bool {
  for (const auto& scheduled_module : current_run) {
    if (scheduled_module.node_name != node_name &&
        drivers_.IsNodeConstrainedToFirstInPipeline(
            scheduled_module.operation_type)) {
      return true;
    }
  }
  return false;
}

auto ElasticSchedulingGraphParser::RemoveUnavailableNodesInThisRun(
    const std::unordered_set<std::string>& available_nodes,
    const std::vector<ScheduledModule>& current_run,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::unordered_set<std::string>& blocked_nodes)
    -> std::unordered_set<std::string> {
  auto resulting_nodes = available_nodes;
  for (const auto& node_name : available_nodes) {
    if (constrained_first_nodes_.find(node_name) !=
        constrained_first_nodes_.end()) {
      for (const auto& module : current_run) {
        for (const auto& [before_node_name, _] :
             graph.at(node_name).before_nodes) {
          if (module.node_name == before_node_name) {
            resulting_nodes.erase(node_name);
          }
        }
      }
    }
    if (drivers_.IsNodeConstrainedToFirstInPipeline(
            graph.at(node_name).operation) &&
        CurrentRunHasFirstModule(current_run, node_name)) {
      resulting_nodes.erase(node_name);
    }
    if (blocked_nodes.find(node_name) != blocked_nodes.end()) {
      resulting_nodes.erase(node_name);
    }
  }
  return resulting_nodes;
}

auto ElasticSchedulingGraphParser::GetMinPositionInCurrentRun(
    const std::vector<ScheduledModule>& current_run,
    const std::string& node_name,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph) -> int {
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
  }
  return 0;
}

auto ElasticSchedulingGraphParser::GetTakenColumns(
    const std::vector<ScheduledModule>& current_run)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> taken_columns;
  taken_columns.reserve(current_run.size());
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
    const std::vector<std::vector<std::string>>& bitstream_start_locations)
    -> std::vector<std::tuple<int, int, int>> {
  std::vector<std::tuple<int, int, int>> all_positions_and_bitstream_indexes;
  for (int start_location_index = min_position;
       start_location_index < bitstream_start_locations.size();
       start_location_index++) {
    for (const auto& bitstream_name :
         bitstream_start_locations.at(start_location_index)) {
      int bitstream_index =
          std::find(hw_library_.at(current_operation)
                        .starting_locations.at(start_location_index)
                        .begin(),
                    hw_library_.at(current_operation)
                        .starting_locations.at(start_location_index)
                        .end(),
                    bitstream_name) -
          hw_library_.at(current_operation)
              .starting_locations.at(start_location_index)
              .begin();
      if (taken_positions.empty()) {
        all_positions_and_bitstream_indexes.emplace_back(
            0, start_location_index, bitstream_index);
      } else {
        auto module_index =
            GetModuleIndex(start_location_index, taken_positions);
        auto end_index = start_location_index +
                         hw_library_.at(current_operation)
                             .bitstream_map.at(bitstream_name)
                             .length -
                         1;
        if ((taken_positions.size() == module_index &&
             taken_positions.at(module_index - 1).second <
                 start_location_index) ||
            (taken_positions.size() != module_index &&
             taken_positions.at(module_index).first > end_index)) {
          all_positions_and_bitstream_indexes.emplace_back(
              module_index, start_location_index, bitstream_index);
        }
      }
    }
  }
  return std::move(all_positions_and_bitstream_indexes);
}

auto ElasticSchedulingGraphParser::GetBitstreamEndFromLibrary(
    int chosen_bitstream_index, int chosen_column_position,
    QueryOperationType current_operation) -> std::pair<std::string, int> {
  /*auto chosen_bitstream_name =
      hw_library.at(current_operation)
          .starting_locations.at(chosen_column_position)
          .at(chosen_bitstream_index);*/
  /*auto end_index = chosen_column_position +
                   hw_library.at(current_operation)
                       .bitstream_map.at(chosen_bitstream_name)
                       .length -
                   1;*/
  return {hw_library_.at(current_operation)
              .starting_locations.at(chosen_column_position)
              .at(chosen_bitstream_index),
          chosen_column_position +
              hw_library_.at(current_operation)
                  .bitstream_map
                  .at(hw_library_.at(current_operation)
                          .starting_locations.at(chosen_column_position)
                          .at(chosen_bitstream_index))
                  .length -
              1};
}

void ElasticSchedulingGraphParser::ReduceSelectionAccordingToHeuristics(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        resulting_module_placements,
    const std::vector<std::vector<ModuleSelection>>& heuristics) {
  std::vector<std::unordered_set<std::pair<int, ScheduledModule>, PairHash>>
      all_selected_placements;
  all_selected_placements.reserve(heuristics.size());
  for (const auto& module_placement_clause : heuristics) {
    auto current_selected_placements = resulting_module_placements;
    for (const auto& placement_selection_function : module_placement_clause) {
      placement_selection_function.SelectAccordingToMode(
          current_selected_placements);
    }
    all_selected_placements.push_back(std::move(current_selected_placements));
  }
  resulting_module_placements.clear();
  for (auto& chosen_module_set : all_selected_placements) {
    resulting_module_placements.merge(chosen_module_set);
  }
}

auto ElasticSchedulingGraphParser::GetChosenModulePlacements(
    const std::string& node_name, QueryOperationType current_operation,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    int min_position, const std::vector<std::pair<int, int>>& taken_positions,
    const std::vector<std::vector<std::string>>& bitstream_start_locations,
    const std::vector<SortedSequence>& processed_table_data,
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        module_placements) -> bool {
  auto available_bitstreams = FindAllAvailableBitstreamsAfterMinPos(
      current_operation, min_position, taken_positions,
      bitstream_start_locations);
  bool modules_found = false;
  if (!available_bitstreams.empty()) {
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash> new_modules;
    for (const auto& [chosen_module_position, chosen_column_position,
                      chosen_bitstream_index] : available_bitstreams) {
      auto [chosen_bitstream, end_index] = GetBitstreamEndFromLibrary(
          chosen_bitstream_index, chosen_column_position, current_operation);
      new_modules.insert({chosen_module_position,
                          {node_name,
                           current_operation,
                           chosen_bitstream,
                           {chosen_column_position, end_index},
                           processed_table_data}});
    }
    if (!new_modules.empty()) {
      statistics_counters_.first += new_modules.size();
      ReduceSelectionAccordingToHeuristics(new_modules, heuristics);
      statistics_counters_.first -= new_modules.size();
      statistics_counters_.second += 1;
      module_placements.merge(new_modules);
      modules_found = true;
    }
  }
  return modules_found;
}

void ElasticSchedulingGraphParser::GetScheduledModulesForNodeAfterPos(
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    int min_position, const std::string& node_name,
    const std::vector<std::pair<int, int>>& taken_positions,
    const std::map<std::string, TableMetadata>& data_tables,
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        module_placements) {
  auto current_operation = graph.at(node_name).operation;
  std::vector<SortedSequence> processed_tables_data;
  // TODO: Change this hack later
  if (!graph.at(node_name).data_tables.at(0).empty()) {
    processed_tables_data =
        data_tables.at(graph.at(node_name).data_tables.at(0)).sorted_status;
  }
  /*std::vector<TableMetadata> processed_tables_data;
  for (const auto& table_name : graph.at(node_name).data_tables) {
    if (table_name.empty()) {
      processed_tables_data.push_back({-1, -1, {}});
    } else {
      processed_tables_data.push_back(data_tables.at(table_name));
    }
  }*/
  auto modules_found = false;
  if (!graph.at(node_name).satisfying_bitstreams.empty() &&
      !heuristics_.first.empty()) {
    modules_found = GetChosenModulePlacements(
        node_name, current_operation, heuristics_.first, min_position,
        taken_positions, graph.at(node_name).satisfying_bitstreams,
        processed_tables_data, module_placements);
  }
  if (!modules_found) {
    GetChosenModulePlacements(
        node_name, current_operation, heuristics_.second, min_position,
        taken_positions,
        hw_library_.at(graph.at(node_name).operation).starting_locations,
        processed_tables_data, module_placements);
  }
}

auto ElasticSchedulingGraphParser::CheckForSkippableSortOperations(
    const std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    const std::map<std::string, TableMetadata>& new_tables,
    const std::string& node_name) -> std::vector<std::string> {
  std::vector<std::string> skipped_nodes;
  auto all_tables_sorted = !std::any_of(
      new_graph.at(node_name).data_tables.begin(),
      new_graph.at(node_name).data_tables.end(),
      [&](const std::string& table_name) {
        return !QuerySchedulingHelper::IsTableSorted(new_tables.at(table_name));
      });
  if (all_tables_sorted) {
    for (const auto& current_next_node_name :
         new_graph.at(node_name).after_nodes) {
      if (!current_next_node_name.empty()) {
        if (drivers_.IsOperationSorting(
                new_graph.at(current_next_node_name).operation)) {
          skipped_nodes.push_back(current_next_node_name);
        }
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
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::vector<int>& missing_utility,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    const std::string& node_name, bool is_node_fully_processed) {
  if (!is_node_fully_processed) {
    new_graph.at(node_name) = graph.at(node_name);
    std::vector<int> new_capacity_values;
    new_capacity_values.reserve(missing_utility.size());
    for (int capacity_parameter_index : missing_utility) {
      new_capacity_values.push_back(std::max(0, capacity_parameter_index));
    }
    new_graph.at(node_name).capacity = new_capacity_values;
  }
}

auto ElasticSchedulingGraphParser::GetResultingTables(
    const std::vector<std::string>& table_names,
    const std::map<std::string, TableMetadata>& tables,
    QueryOperationType operation) -> std::vector<std::string> {
  if (drivers_.IsInputSupposedToBeSorted(operation)) {
    for (const auto& table_name : table_names) {
      if (!QuerySchedulingHelper::IsTableSorted(tables.at(table_name))) {
        throw std::runtime_error("Table should be sorted!");
      }
    }
  }
  return drivers_.GetResultingTables(operation, table_names, tables);
}

void ElasticSchedulingGraphParser::UpdateNextNodeTables(
    const std::unordered_map<std::string, SchedulingQueryNode>& /*graph*/,
    const std::string& node_name,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    const std::vector<std::string>& skipped_nodes,
    const std::vector<std::string>& resulting_tables) {
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
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::string& bitstream,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::string& node_name, const std::vector<int>& capacity,
    QueryOperationType operation,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    std::map<std::string, TableMetadata>& new_data_tables)
    -> std::pair<bool, std::vector<std::string>> {
  std::vector<std::string> skipped_nodes;
  bool is_node_fully_processed = false;
  if (drivers_.IsOperationSorting(operation)) {
    is_node_fully_processed = drivers_.UpdateDataTable(
        operation,
        hw_library_.at(operation).bitstream_map.at(bitstream).capacity,
        graph.at(node_name).data_tables, data_tables, new_data_tables);
    skipped_nodes =
        CheckForSkippableSortOperations(new_graph, new_data_tables, node_name);
  } else {
    std::vector<int> missing_utility;
    is_node_fully_processed = FindMissingUtility(
        hw_library_.at(operation).bitstream_map.at(bitstream).capacity,
        missing_utility, capacity);
    UpdateGraphCapacities(graph, missing_utility, new_graph, node_name,
                          is_node_fully_processed);
  }
  if (is_node_fully_processed) {
    auto resulting_table = GetResultingTables(graph.at(node_name).data_tables,
                                              new_data_tables, operation);
    UpdateNextNodeTables(graph, node_name, new_graph, skipped_nodes,
                         resulting_table);
  }
  return {is_node_fully_processed, skipped_nodes};
}

auto ElasticSchedulingGraphParser::CreateNewAvailableNodes(
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::unordered_set<std::string>& available_nodes,
    std::unordered_set<std::string>& processed_nodes,
    const std::string& node_name, bool satisfied_requirements) {
  if (satisfied_requirements) {
    available_nodes.erase(node_name);
    processed_nodes.insert(node_name);
    QuerySchedulingHelper::GetNewAvailableNodesAfterSchedulingGivenNode(
        node_name, processed_nodes, graph, available_nodes);
  }
}

auto ElasticSchedulingGraphParser::IsTableEqualForGivenNode(
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    const std::string& node_name,
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
    const std::string& node_name,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    std::unordered_set<std::string>& new_available_nodes,
    const std::map<std::string, TableMetadata>& data_tables,
    std::map<std::string, TableMetadata>& new_tables,
    const std::unordered_set<std::string>& new_processed_nodes) {
  if (std::any_of(
          new_graph.begin(), new_graph.end(), [&](const auto& map_entry) {
            return graph.at(map_entry.first).capacity !=
                       new_graph.at(map_entry.first).capacity ||
                   !IsTableEqualForGivenNode(graph, new_graph, map_entry.first,
                                             data_tables, new_tables);
          })) {
    // For performance remove these checks! These checks don't work anyways as
    // tables can be changed with node removal.
    /*std::vector<std::vector<std::string>> previous_tables;
    for (const auto& [_, node] : graph) {
      previous_tables.push_back(node.data_tables);
    }*/
    PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
        hw_library_, new_graph, new_tables, drivers_, new_available_nodes,
        new_processed_nodes);
    /*std::vector<std::vector<std::string>> current_tables;
    for (const auto& [_, node] : graph) {
      current_tables.push_back(node.data_tables);
    }
    if (previous_tables != current_tables) {
      throw std::runtime_error("Something went wrong!");
    }*/
  } else {
    if (std::any_of(
            graph.at(node_name).after_nodes.begin(),
            graph.at(node_name).after_nodes.end(),
            [&](const std::string& next_node_name) {
              return (!next_node_name.empty() &&
                      new_graph.find(next_node_name) == new_graph.end()) ||
                     (!next_node_name.empty() &&
                      !IsTableEqualForGivenNode(graph, new_graph,
                                                next_node_name, data_tables,
                                                new_tables));
            })) {
      /*std::vector<std::vector<std::string>> previous_tables;
      for (const auto& [_, node] : graph) {
        previous_tables.push_back(node.data_tables);
      }*/
      PreSchedulingProcessor::AddSatisfyingBitstreamLocationsToGraph(
          hw_library_, new_graph, new_tables, drivers_, new_available_nodes,
          new_processed_nodes);
      /*std::vector<std::vector<std::string>> current_tables;
      for (const auto& [_, node] : graph) {
        current_tables.push_back(node.data_tables);
      }
      if (previous_tables != current_tables) {
        throw std::runtime_error("Something went wrong!");
      }*/
    }
  }
}

void ElasticSchedulingGraphParser::FindDataSensitiveNodeNames(
    const std::string& node_name,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::unordered_set<std::string>& new_next_run_blocked_nodes) {
  for (const auto& next_node_name : graph.at(node_name).after_nodes) {
    if (!next_node_name.empty()) {
      if (drivers_.IsOperationDataSensitive(
              graph.at(next_node_name).operation)) {
        new_next_run_blocked_nodes.insert(next_node_name);
      }
      FindDataSensitiveNodeNames(next_node_name, graph,
                                 new_next_run_blocked_nodes);
    }
  }
}

void ElasticSchedulingGraphParser::GetNewBlockedNodes(
    std::unordered_set<std::string>& next_run_blocked_nodes,
    const ScheduledModule& module_placement,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph) {
  if (drivers_.IsOperationReducingData(module_placement.operation_type)) {
    FindDataSensitiveNodeNames(module_placement.node_name, graph,
                               next_run_blocked_nodes);
  }
}

auto ElasticSchedulingGraphParser::GetNewStreamedDataSize(
    const std::vector<ScheduledModule>& current_run,
    const std::string& node_name,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph) -> int {
  std::vector<std::string> before_node_names;
  for (const auto& before_stream : graph.at(node_name).before_nodes) {
    before_node_names.push_back(before_stream.first);
  }
  auto required_tables = graph.at(node_name).data_tables;
  for (const auto& placement : current_run) {
    auto before_node_search =
        std::find(before_node_names.begin(), before_node_names.end(),
                  placement.node_name);
    if (before_node_search != before_node_names.end()) {
      required_tables[before_node_search - before_node_names.begin()] = "";
    }
  }

  int streamed_data_size = 0;
  for (const auto& table_name : required_tables) {
    if (!table_name.empty()) {
      // Record size is in 4 byte words
      streamed_data_size += data_tables.at(table_name).record_count *
                            data_tables.at(table_name).record_size * 4;
    }
  }
  return streamed_data_size;
}

auto ElasticSchedulingGraphParser::IsSubsetOf(
    const std::unordered_set<std::string>& a,
    const std::unordered_set<std::string>& b) -> bool {
  // return true if all members of a are also in b
  if (a.size() > b.size()) {
    return false;
  }

  auto const not_found = b.end();
  for (auto const& element : a) {
    if (b.find(element) == not_found) {
      return false;
    }
  }

  return true;
}

void ElasticSchedulingGraphParser::FindNextModulePlacement(
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    std::unordered_set<std::string>& new_available_nodes,
    const ScheduledModule& module_placement, const std::string& node_name,
    std::unordered_set<std::string>& new_processed_nodes,
    const std::map<std::string, TableMetadata>& data_tables,
    std::map<std::string, TableMetadata>& new_data_tables,
    std::unordered_set<std::string>& new_next_run_blocked_nodes) {
  auto [satisfied_requirements, skipped_node_names] =
      UpdateGraph(graph, module_placement.bitstream, data_tables, node_name,
                  graph.at(node_name).capacity, graph.at(node_name).operation,
                  new_graph, new_data_tables);

  CreateNewAvailableNodes(graph, new_available_nodes, new_processed_nodes,
                          node_name, satisfied_requirements);

  for (const auto& skipped_node_name : skipped_node_names) {
    if (new_available_nodes.find(skipped_node_name) ==
        new_available_nodes.end()) {
      throw std::runtime_error("Skipped nodes marked in the wrong order!");
    }
    CreateNewAvailableNodes(graph, new_available_nodes, new_processed_nodes,
                            skipped_node_name, satisfied_requirements);
  }

  UpdateSatisfyingBitstreamsList(node_name, graph, new_graph,
                                 new_available_nodes, data_tables,
                                 new_data_tables, new_processed_nodes);

  GetNewBlockedNodes(new_next_run_blocked_nodes, module_placement, graph);
}

void ElasticSchedulingGraphParser::PlaceNodesRecursively(
    const std::unordered_set<std::string>& available_nodes,
    const std::unordered_set<std::string>& processed_nodes,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::vector<ScheduledModule>& current_run,
    std::vector<std::vector<ScheduledModule>> current_plan,
    const std::map<std::string, TableMetadata>& data_tables,
    const std::unordered_set<std::string>& blocked_nodes,
    const std::unordered_set<std::string>& next_run_blocked_nodes,
    const int streamed_data_size) {
  if (trigger_timeout_) {
    throw TimeLimitException("Timeout");
  }
  if (!(current_plan.size() > min_runs && use_max_runs_cap_) &&
      !trigger_timeout_) {
    if (!available_nodes.empty() &&
        !IsSubsetOf(available_nodes, blocked_nodes)) {
      auto available_nodes_in_this_run = RemoveUnavailableNodesInThisRun(
          available_nodes, current_run, graph, blocked_nodes);
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>
          available_module_placements;
      for (const auto& node_name : available_nodes) {
        if (available_nodes_in_this_run.find(node_name) !=
            available_nodes_in_this_run.end()) {
          GetScheduledModulesForNodeAfterPos(
              graph, GetMinPositionInCurrentRun(current_run, node_name, graph),
              node_name, GetTakenColumns(current_run), data_tables,
              available_module_placements);
        }
      }
      if (!available_module_placements.empty()) {
        for (const auto& [module_index, module_placement] :
             available_module_placements) {
          auto new_current_run = current_run;
          new_current_run.insert(new_current_run.begin() + module_index,
                                 module_placement);
          auto new_streamed_data_size = streamed_data_size;
          new_streamed_data_size += GetNewStreamedDataSize(
              current_run, module_placement.node_name, data_tables, graph);

          std::unordered_set<std::string> new_available_nodes = available_nodes;
          std::unordered_set<std::string> new_processed_nodes = processed_nodes;
          std::map<std::string, TableMetadata> new_data_tables = data_tables;
          std::unordered_map<std::string, SchedulingQueryNode> new_graph =
              graph;
          std::unordered_set<std::string> new_next_run_blocked_nodes =
              next_run_blocked_nodes;

          FindNextModulePlacement(graph, new_graph, new_available_nodes,
                                  module_placement, module_placement.node_name,
                                  new_processed_nodes, data_tables,
                                  new_data_tables, new_next_run_blocked_nodes);

          PlaceNodesRecursively(
              std::move(new_available_nodes), std::move(new_processed_nodes),
              std::move(new_graph), std::move(new_current_run), current_plan,
              std::move(new_data_tables), blocked_nodes,
              std::move(new_next_run_blocked_nodes),
              std::move(new_streamed_data_size));
        }
      }
      if ((available_module_placements.empty() || !reduce_single_runs_) &&
          !current_run.empty()) {
        for (const auto& node_name : available_nodes) {
          if (blocked_nodes.find(node_name) == blocked_nodes.end()) {
            auto new_blocked_nodes = blocked_nodes;
            new_blocked_nodes.insert(next_run_blocked_nodes.begin(),
                                     next_run_blocked_nodes.end());
            if (new_blocked_nodes.find(node_name) != new_blocked_nodes.end()) {
              PlaceNodesRecursively(available_nodes, processed_nodes, graph,
                                    current_run, current_plan, data_tables,
                                    std::move(new_blocked_nodes), {},
                                    streamed_data_size);
              return;
            }

            auto new_current_plan = current_plan;
            if (!current_run.empty()) {
              new_current_plan.push_back(current_run);
            }
            GetScheduledModulesForNodeAfterPos(graph, 0, node_name, {},
                                               data_tables,
                                               available_module_placements);
            if (!available_module_placements.empty()) {
              for (const auto& [_, module_placement] :
                   available_module_placements) {
                auto new_streamed_data_size = streamed_data_size;
                new_streamed_data_size += GetNewStreamedDataSize(
                    current_run, node_name, data_tables, graph);

                std::unordered_set<std::string> new_available_nodes =
                    available_nodes;
                std::unordered_set<std::string> new_processed_nodes =
                    processed_nodes;
                std::map<std::string, TableMetadata> new_data_tables =
                    data_tables;
                std::unordered_set<std::string> new_next_run_blocked_nodes;
                std::unordered_map<std::string, SchedulingQueryNode> new_graph =
                    graph;

                FindNextModulePlacement(
                    graph, new_graph, new_available_nodes, module_placement,
                    node_name, new_processed_nodes, data_tables,
                    new_data_tables, new_next_run_blocked_nodes);

                PlaceNodesRecursively(
                    std::move(new_available_nodes),
                    std::move(new_processed_nodes), std::move(new_graph),
                    {module_placement}, std::move(new_current_plan),
                    std::move(new_data_tables), std::move(new_blocked_nodes),
                    std::move(new_next_run_blocked_nodes),
                    std::move(new_streamed_data_size));
              }
            } else {
              throw std::runtime_error(
                  "Should be able to place nodes in an empty run!");
            }
          }
        }
      }
    } else {
      current_plan.push_back(current_run);
      ExecutionPlanSchedulingData current_scheduling_data = {
          processed_nodes, available_nodes, graph, data_tables,
          streamed_data_size};
      if (const auto& [it, inserted] = resulting_plan_.try_emplace(
              current_plan, current_scheduling_data);
          inserted) {
        if (current_plan.size() < min_runs) {
          min_runs = current_plan.size();
        }
      }
      if (std::chrono::system_clock::now() > time_limit_) {
        trigger_timeout_ = true;
      }
    }
  }
}

auto ElasticSchedulingGraphParser::GetTimeoutStatus() -> bool {
  return trigger_timeout_;
}

auto ElasticSchedulingGraphParser::GetResultingPlan()
    -> std::map<std::vector<std::vector<ScheduledModule>>,
                ExecutionPlanSchedulingData> {
  return resulting_plan_;
}

auto ElasticSchedulingGraphParser::GetStats() -> std::pair<int, int> {
  return statistics_counters_;
}
