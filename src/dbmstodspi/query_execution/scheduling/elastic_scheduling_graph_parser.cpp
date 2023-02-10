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
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "pre_scheduling_processor.hpp"
#include "query_scheduling_helper.hpp"
#include "table_data.hpp"
#include "time_limit_execption.hpp"

using orkhestrafs::dbmstodspi::ElasticSchedulingGraphParser;
using orkhestrafs::dbmstodspi::PairHash;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;
using orkhestrafs::dbmstodspi::TimeLimitException;
#ifdef FPGA_AVAILALBLE
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
#endif  // FPGA_AVAILALBLE

void ElasticSchedulingGraphParser::PreprocessNodes(
    std::unordered_set<std::string>& available_nodes,
    std::unordered_set<std::string>& processed_nodes,
    std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& data_tables) {
  pre_scheduler_.AddSatisfyingBitstreamLocationsToGraph(
      graph, data_tables, available_nodes, processed_nodes);
}

auto ElasticSchedulingGraphParser::CurrentRunHasFirstModule(
    const std::vector<ScheduledModule>& current_run,
    const std::string& node_name, const QueryOperationType operation_type)
    -> bool {
  // You can't have to merge sorters being first as they both can't work on
  // stream 0.
  if (drivers_.IsNodeConstrainedToFirstInPipeline(operation_type)) {
    for (const auto& scheduled_module : current_run) {
      if (scheduled_module.node_name != node_name &&
          drivers_.IsNodeConstrainedToFirstInPipeline(
              scheduled_module.operation_type)) {
        return true;
      }
    }
  }
  return false;
}

auto ElasticSchedulingGraphParser::RemoveUnavailableNodesInThisRun(
    const std::unordered_set<std::string>& available_nodes,
    const std::vector<ScheduledModule>& current_run,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::unordered_set<std::string>& blocked_nodes, 
    const std::map<std::string, TableMetadata>& data_tables)
    -> std::unordered_set<std::string> {
  auto resulting_nodes = available_nodes;
  for (const auto& node_name : available_nodes) {
    // Does the current run have something planned already such that you can't
    // place a node into first location
    if (constrained_first_nodes_.find(node_name) !=
        constrained_first_nodes_.end()) {
      if (CurrentRunHasFirstModule(current_run, node_name,
                                   graph.at(node_name).operation)) {
        resulting_nodes.erase(node_name);
      } else {
        // Is there a parent node in the current run already.
        if (std::any_of(
                current_run.begin(), current_run.end(),
                [&](const auto& cur_module) {
                  return std::find_if(
                             graph.at(node_name).before_nodes.begin(),
                             graph.at(node_name).before_nodes.end(),
                             [&](const auto& before_pointer) {
                                       return before_pointer.first ==
                                              cur_module.node_name;
                             }) != graph.at(node_name).before_nodes.end();
                })) {
          resulting_nodes.erase(node_name);
        }
      }
    }
    if (blocked_nodes.find(node_name) != blocked_nodes.end()) {
      resulting_nodes.erase(node_name);
    }

  //  auto current_operation = graph.at(node_name).operation;

  //  if (std::any_of(
  //          current_run.begin(), current_run.end(), [&](const auto& cur_module) {
  //            return current_operation == cur_module.operation_type;})) {
  //    resulting_nodes.erase(node_name);
  //  }

  //  if (current_operation == QueryOperationType::kAddition) {
  //    if (std::any_of(current_run.begin(), current_run.end(),
  //                    [](const auto& cur_module) {
  //                      return cur_module.operation_type == QueryOperationType::kMergeSort;
  //                    })) {
  //      resulting_nodes.erase(node_name);
  //    }
  //  } else if (current_operation == QueryOperationType::kAggregationSum) {
  //      // Do nothing
  //  } else if (current_operation == QueryOperationType::kFilter) {
  //      // Do nothing
  //  } else if (current_operation == QueryOperationType::kJoin) {
  //    if (std::any_of(current_run.begin(), current_run.end(),
  //                    [](const auto& cur_module) {
  //                      return cur_module.operation_type ==
  //                                 QueryOperationType::kMultiplication ||
  //                             cur_module.operation_type ==
  //                                 QueryOperationType::kLinearSort;
  //                    })) {
  //      resulting_nodes.erase(node_name);
  //    }
  //  } else if (current_operation == QueryOperationType::kLinearSort) {
  //    if (std::any_of(current_run.begin(), current_run.end(),
  //                    [](const auto& cur_module) {
  //                      return cur_module.operation_type ==
  //                                 QueryOperationType::kMergeSort ||
  //                             cur_module.operation_type ==
  //                                 QueryOperationType::kJoin;
  //                      ;
  //                    })) {
  //      resulting_nodes.erase(node_name);
  //    }
  //  } else if (current_operation == QueryOperationType::kMergeSort) {
  //    if (std::any_of(current_run.begin(), current_run.end(),
  //                    [](const auto& cur_module) {
  //                      return cur_module.operation_type ==
  //                                 QueryOperationType::kAddition ||
  //                             cur_module.operation_type ==
  //                                 QueryOperationType::kMultiplication ||
  //                             cur_module.operation_type ==
  //                                 QueryOperationType::kLinearSort;
  //                      ;
  //                      ;
  //                    })) {
  //      resulting_nodes.erase(node_name);
  //    }
  //  } else if (current_operation == QueryOperationType::kMultiplication) {
  //    if (std::any_of(current_run.begin(), current_run.end(),
  //                    [](const auto& cur_module) {
  //                      return cur_module.operation_type ==
  //                                 QueryOperationType::kMergeSort ||
  //                             cur_module.operation_type ==
  //                                 QueryOperationType::kJoin;
  //                      ;
  //                    })) {
  //      resulting_nodes.erase(node_name);
  //    }
  //  }
  }

  // Is input table been processed?
  return resulting_nodes;
}

auto ElasticSchedulingGraphParser::GetMinPositionInCurrentRun(
    const std::vector<ScheduledModule>& current_run,
    const std::string& node_name,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph) -> int {
  /*std::vector<ScheduledModule> currently_scheduled_prereq_nodes;*/
  int current_min = -1;
  for (const auto& [previous_node_name, _] : graph.at(node_name).before_nodes) {
    for (const auto& module_placement : current_run) {
      if (previous_node_name == module_placement.node_name) {
        if (module_placement.position.second > current_min) {
          current_min = module_placement.position.second;
        }
      }
    }
  }
  return current_min + 1;
  //if (!currently_scheduled_prereq_nodes.empty()) {
  //  
  //  for (const auto& module_placement : currently_scheduled_prereq_nodes) {
  //    
  //  }
  //  // Assuming current min isn't 0 or negative somehow.
  //  
  //}
  //return 0;
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
  auto original_placements = resulting_module_placements;
  resulting_module_placements.clear();
  std::unordered_set<std::pair<int, ScheduledModule>, PairHash>
      current_selected_placements;
  for (const auto& module_placement_clause : heuristics) {
    current_selected_placements = original_placements;
    for (const auto& placement_selection_function : module_placement_clause) {
      placement_selection_function.SelectAccordingToMode(
          current_selected_placements);
    }
    resulting_module_placements.merge(current_selected_placements);
  }
}

auto ElasticSchedulingGraphParser::GetChosenModulePlacements(
    const std::string& node_name, QueryOperationType current_operation,
    const std::vector<std::vector<ModuleSelection>>& heuristics,
    int min_position, const std::vector<std::pair<int, int>>& taken_positions,
    const std::vector<std::vector<std::string>>& bitstream_start_locations,
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        module_placements,
    bool is_composed) -> bool {
  // Do I need to construct this every time?
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
                           is_composed}});
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
    const std::vector<ScheduledModule>& current_run,
    const std::string& node_name,
    const std::map<std::string, TableMetadata>& data_tables,
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        module_placements) {
  // Get current query
  std::string current_query = node_name;
  for (const auto& module : current_run) {
    current_query += module.bitstream;
    if (use_single_runs_) {
      return;
    }
  }

  // Find placements
  if (saved_placements_.find(current_query) == saved_placements_.end()) {
    // Not found
    bool is_composed = std::any_of(
        current_run.begin(), current_run.end(),
        [&](const auto& module) { return module.node_name == node_name; });
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>
        found_placements;
    GetScheduledModulesForNodeAfterPosOrig(
        graph, GetMinPositionInCurrentRun(current_run, node_name, graph),
        node_name, GetTakenColumns(current_run), data_tables, found_placements,
        is_composed);
    saved_placements_.insert(
        {current_query, std::move(found_placements)});
    //module_placements.merge(found_placements);
  }
    
  //} else {
    // "Cached"
    /*std::vector<std::pair<int, ScheduledModule>> found_placements(
        search->second.begin(), search->second.end());*/
    // Composed status currently is saved as well!
    /*bool is_composed = std::any_of(
        current_run.begin(), current_run.end(),
        [&](const auto& module) { return module.node_name == node_name });
    for (auto& new_module_placement : found_placements) {
      new_module_placement.second.is_composed = is_composed;
    }*/
  //  module_placements.insert(search->second.begin(), search->second.end());
  //}
  module_placements.insert(saved_placements_[current_query].begin(),
                           saved_placements_[current_query].end());
}

void ElasticSchedulingGraphParser::GetScheduledModulesForNodeAfterPosOrig(
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    int min_position, const std::string& node_name,
    const std::vector<std::pair<int, int>>& taken_positions,
    const std::map<std::string, TableMetadata>& /*data_tables*/,
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        module_placements,
    bool is_composed) {
  auto modules_found = false;
  if (!graph.at(node_name).satisfying_bitstreams.empty() &&
      !heuristics_.first.empty()) {
    modules_found = GetChosenModulePlacements(
        node_name, graph.at(node_name).operation, heuristics_.first,
        min_position, taken_positions,
        graph.at(node_name).satisfying_bitstreams, module_placements,
        is_composed);
  }
  if (!modules_found) {
    if (drivers_.IsIncompleteOperationSupported(graph.at(node_name).operation)){
      modules_found = GetChosenModulePlacements(
          node_name, graph.at(node_name).operation, heuristics_.second,
          min_position, taken_positions,
          hw_library_.at(graph.at(node_name).operation).starting_locations,
          module_placements, is_composed);
    } else if (!graph.at(node_name).satisfying_bitstreams.empty() && heuristics_.first.empty()) {
      modules_found = GetChosenModulePlacements(
          node_name, graph.at(node_name).operation, heuristics_.second,
          min_position, taken_positions,
          graph.at(node_name).satisfying_bitstreams,
          module_placements, is_composed);
    }
  }
  if (!modules_found && taken_positions.empty()) {
    throw std::runtime_error("Unable to find a valid module for node:" + node_name);
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
    std::vector<int>& /*missing_capacity*/,
    const std::vector<int>& node_capacity,
    QueryOperationType /*operation_type*/, bool /*is_composed*/) -> bool {
  // You can get rid of this error check one day and make it module specific.
  if (bitstream_capacity.size() != node_capacity.size()) {
    throw std::runtime_error("Capacity parameters don't match!");
  }
  throw std::runtime_error("Not implemented method!");
}

void ElasticSchedulingGraphParser::UpdateGraphCapacities(
    const std::vector<int>& missing_utility,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    const std::string& node_name, bool is_node_fully_processed) {
  if (!is_node_fully_processed) {
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
    const std::string& node_name,
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    const std::vector<std::string>& resulting_tables) {
  QuerySchedulingHelper::AddNewTableToNextNodes(new_graph, node_name,
                                                resulting_tables);
  //  new_graph.erase(node_name);
  //  for (const auto& skipped_node : skipped_nodes) {
  //    QuerySchedulingHelper::AddNewTableToNextNodes(new_graph, skipped_node,
  //                                                  resulting_tables);
  //    new_graph.erase(skipped_node);
  //  }
}

auto ElasticSchedulingGraphParser::UpdateGraphCapacitiesAndTables(
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    std::map<std::string, TableMetadata>& new_data_tables,
    const std::string& bitstream, const std::string& node_name,
    const std::vector<int>& capacity, QueryOperationType operation,
    bool is_composed) -> bool {
  bool is_node_fully_processed = false;
  if (operation == QueryOperationType::kLinearSort) {
    // Just linear sort. It always gets fully processed and no need to look at
    // capacity. Make it more generic in the future!
    // TODO(Kaspar): Assuming single next node.
    is_node_fully_processed = drivers_.UpdateDataTable(
        operation,
        hw_library_.at(operation).bitstream_map.at(bitstream).capacity,
        new_graph.at(node_name).node_ptr->given_output_data_definition_files,
        new_data_tables);
    const auto& next_node_name = new_graph.at(node_name).after_nodes.front();
    if (!next_node_name.empty()) {
      if (new_graph.at(next_node_name).operation ==
          QueryOperationType::kMergeSort) {
        auto required_merge_capacity = drivers_.GetWorstCaseNodeCapacity(
            operation,
            hw_library_.at(operation).bitstream_map.at(bitstream).capacity,
            new_graph.at(node_name).data_tables, new_data_tables,
            new_graph.at(next_node_name).operation);
        new_graph[next_node_name].capacity = required_merge_capacity;
      } else {
        // Assume the sort has been skipped and there isn't a sort later in the
        // graph.
        // TODO(Kaspar): Fix this assumption!
      }
    }

    //    if (is_node_fully_processed) {
    //      skipped_nodes = CheckForSkippableSortOperations(
    //          new_graph, new_data_tables, node_name);
    //    }
  } else {
    std::vector<int> missing_utility;
    auto placed_module_capacity =
        hw_library_.at(operation).bitstream_map.at(bitstream).capacity;
    is_node_fully_processed = drivers_.SetMissingFunctionalCapacity(
        hw_library_.at(operation).bitstream_map.at(bitstream).capacity,
        missing_utility, capacity, is_composed, operation);
    UpdateGraphCapacities(missing_utility, new_graph, node_name,
                          is_node_fully_processed);
  }
  
  //if (is_node_fully_processed) {
  //  for (const auto& processed_table_name :
  //       new_graph.at(node_name).data_tables) {
  //    new_data_tables.at(processed_table_name).is_finished = true;
  //  }
    // No need for this anymore! Maybe add 90% filtering here later
    /*auto resulting_table = GetResultingTables(
        new_graph.at(node_name).data_tables, new_data_tables, operation);
    UpdateNextNodeTables(node_name, new_graph, resulting_table);*/
  //}
  return is_node_fully_processed;
}

auto ElasticSchedulingGraphParser::CreateNewAvailableNodes(
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::unordered_set<std::string>& available_nodes,
    std::unordered_set<std::string>& processed_nodes,
    const std::string& node_name) {
  available_nodes.erase(node_name);
  processed_nodes.insert(node_name);
  QuerySchedulingHelper::UpdateAvailableNodesAfterSchedulingGivenNode(
      node_name, processed_nodes, graph, available_nodes);
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
                    [](const int& l, const int& r) { return l == r; })) {
      return true;
    }
  }
  return false;
}

void ElasticSchedulingGraphParser::
    UpdateAvailableNodesAndSatisfyingBitstreamsList(
        const std::string& node_name,
        std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
        std::unordered_set<std::string>& new_available_nodes,
        std::map<std::string, TableMetadata>& new_tables,
        std::unordered_set<std::string>& new_processed_nodes,
        QueryOperationType operation, bool satisfied_requirements,
        std::unordered_set<std::string>& new_next_run_blocked_nodes,
        const std::unordered_set<std::string>& blocked_nodes) {
  // If sorting module and not finished - Redo itself; If sorting module and
  // finished - Redo next node.
  if (drivers_.IsOperationSorting(operation)) {
    if (satisfied_requirements) {
      new_processed_nodes.insert(node_name);
      new_available_nodes.erase(node_name);
      auto immediate_new_available_nodes =
          QuerySchedulingHelper::GetNewAvailableNodesAfterSchedulingGivenNode(
              node_name, new_processed_nodes, new_graph);
      pre_scheduler_.AddSatisfyingBitstreamLocationsToGraph(
          new_graph, new_tables, immediate_new_available_nodes,
          new_processed_nodes);
      new_available_nodes.merge(immediate_new_available_nodes);
      new_graph.erase(node_name);
    } else {
      pre_scheduler_.UpdateOnlySatisfyingBitstreams(node_name, new_graph,
                                                    new_tables);
    }
  } else if (satisfied_requirements) {
    CreateNewAvailableNodes(new_graph, new_available_nodes, new_processed_nodes,
                            node_name);
    // TODO(Kaspar): Create a list of nodes that have been checked for this
    // already - perhaps
    GetNewBlockedNodes(new_next_run_blocked_nodes, new_graph, operation,
                       node_name, blocked_nodes);
    new_graph.erase(node_name);
  }
}

void ElasticSchedulingGraphParser::FindDataSensitiveNodeNames(
    const std::string& node_name,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::unordered_set<std::string>& new_next_run_blocked_nodes,
    const std::unordered_set<std::string>& blocked_nodes) {
  for (const auto& next_node_name : graph.at(node_name).after_nodes) {
    if (!next_node_name.empty()) {
      if (drivers_.IsOperationDataSensitive(
              graph.at(next_node_name).operation)) {
        if (blocked_nodes.find(next_node_name) == blocked_nodes.end() &&
            new_next_run_blocked_nodes.find(next_node_name) ==
                new_next_run_blocked_nodes.end()) {
          new_next_run_blocked_nodes.insert(next_node_name);
          FindDataSensitiveNodeNames(next_node_name, graph,
                                     new_next_run_blocked_nodes, blocked_nodes);
        }
        // Else do nothing.
      } else {
        FindDataSensitiveNodeNames(next_node_name, graph,
                                   new_next_run_blocked_nodes, blocked_nodes);
      }
    }
  }
}

// We have a problem over here.
// TODO(Kaspar): Always when a filter or join gets placed it goes through the
// whole thing.
void ElasticSchedulingGraphParser::GetNewBlockedNodes(
    std::unordered_set<std::string>& next_run_blocked_nodes,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    QueryOperationType operation, const std::string& node_name,
    const std::unordered_set<std::string>& blocked_nodes) {
  if (drivers_.IsOperationReducingData(operation)) {
    FindDataSensitiveNodeNames(node_name, graph, next_run_blocked_nodes,
                               blocked_nodes);
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
  auto operation_type = graph.at(node_name).operation;
  int streamed_data_size = 0;
  for (const auto& table_name : required_tables) {
    if (!table_name.empty()) {
      if (operation_type == QueryOperationType::kMergeSort){
        streamed_data_size += graph.at(node_name).capacity.front() * data_tables.at(table_name).sorted_status.at(2) * data_tables.at(table_name).record_size * 4;
      }
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

  return std::all_of(a.begin(), a.end(), [&](const auto& a_string) {
    return b.find(a_string) != b.end();
  });
}

void ElasticSchedulingGraphParser::UpdateGraphAndTableValuesGivenPlacement(
    std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
    std::unordered_set<std::string>& new_available_nodes,
    const ScheduledModule& module_placement,
    std::unordered_set<std::string>& new_processed_nodes,
    std::map<std::string, TableMetadata>& new_data_tables,
    std::unordered_set<std::string>& new_next_run_blocked_nodes,
    const std::unordered_set<std::string>& blocked_nodes) {
  auto satisfied_requirements = UpdateGraphCapacitiesAndTables(
      new_graph, new_data_tables, module_placement.bitstream,
      module_placement.node_name,
      new_graph.at(module_placement.node_name).capacity,
      new_graph.at(module_placement.node_name).operation,
      module_placement.is_composed);

  UpdateAvailableNodesAndSatisfyingBitstreamsList(
      module_placement.node_name, new_graph, new_available_nodes,
      new_data_tables, new_processed_nodes,
      new_graph.at(module_placement.node_name).operation,
      satisfied_requirements, new_next_run_blocked_nodes, blocked_nodes);
}

void ElasticSchedulingGraphParser::GetAllAvailableModulePlacementsInCurrentRun(
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
        available_module_placements,
    const std::unordered_set<std::string>& available_nodes,
    const std::vector<ScheduledModule>& current_run,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    const std::unordered_set<std::string>& blocked_nodes,
    const std::map<std::string, TableMetadata>& data_tables) {

  auto available_nodes_in_this_run = RemoveUnavailableNodesInThisRun(
      available_nodes, current_run, graph, blocked_nodes, data_tables);


  for (const auto& node_name : available_nodes_in_this_run) {
    GetScheduledModulesForNodeAfterPos(graph, current_run, node_name,
                                       data_tables,
                                       available_module_placements);
  }

  if (prioritise_children_) {
    std::unordered_set<std::pair<int, ScheduledModule>, PairHash> result;
    for (const auto& [location, available_module] :
         available_module_placements) {
      const auto& node = graph.at(available_module.node_name);
      for (const auto& [previous_node_name, _] : node.before_nodes) {
        for (const auto& placed_module : current_run) {
          if (placed_module.node_name == previous_node_name) {
            result.insert({location, available_module});
          }
        }
      }
    }
    if (!result.empty()) {
      available_module_placements.clear();
      available_module_placements = std::move(result);
    }
  }
}

void ElasticSchedulingGraphParser::AddPlanToAllPlansAndMeasureTime(
    const std::vector<std::vector<ScheduledModule>>& current_plan,
    const std::unordered_set<std::string>& processed_nodes,
    const std::map<std::string, TableMetadata>& data_tables,
    int streamed_data_size) {
  ExecutionPlanSchedulingData current_scheduling_data = {
      processed_nodes, data_tables, streamed_data_size};
  if (const auto& [it, inserted] =
          resulting_plan_.try_emplace(current_plan, current_scheduling_data);
      inserted) {
    if (current_plan.size() < min_runs_) {
      min_runs_ = current_plan.size();
    }
  }
  if (std::chrono::system_clock::now() > time_limit_) {
    trigger_timeout_ = true;
  }
}

void ElasticSchedulingGraphParser::PlaceNodesRecursively(
    std::unordered_set<std::string> available_nodes,
    std::unordered_set<std::string> processed_nodes,
    std::unordered_map<std::string, SchedulingQueryNode> graph,
    std::vector<ScheduledModule> current_run,
    std::vector<std::vector<ScheduledModule>> current_plan,
    std::map<std::string, TableMetadata> data_tables,
    std::unordered_set<std::string> blocked_nodes,
    std::unordered_set<std::string> next_run_blocked_nodes,
    int streamed_data_size) {
  // TODO(Kaspar): Potentially check for timeouts more often
  if (trigger_timeout_) {
    throw TimeLimitException("Timeout");
  }
  std::unordered_set<std::pair<int, ScheduledModule>, PairHash>
      available_module_placements;
  while (!available_nodes.empty() &&
         !IsSubsetOf(available_nodes, blocked_nodes)) {
    GetAllAvailableModulePlacementsInCurrentRun(
        available_module_placements, available_nodes, current_run, graph,
        blocked_nodes, data_tables);
    // Start planning a new run if we can't find any new valid placements
    if (available_module_placements.empty()) {
      if (current_run.empty()) {
        // Empty runs could be useful for crossbar usage in the future!
        // Can occur if no fitting modules were found
        throw std::runtime_error("Can't use an empty run at the moment!");
      }
      current_plan.push_back(std::move(current_run));
      if (use_max_runs_cap_ && current_plan.size() > min_runs_) {
        return;
      }
      current_run.clear();
      blocked_nodes.merge(next_run_blocked_nodes);
      next_run_blocked_nodes.clear();
    } else {
      // Get the placement we are going to place in this branch
      auto current_placement =
          std::move(available_module_placements
                        .extract(available_module_placements.begin())
                        .value());
      // If there are still other placements to consider do them in different
      // recursion branches.
      if (!available_module_placements.empty()) {
        for (const auto& [module_index, module_placement] :
             available_module_placements) {
          // Make new variables
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

          // Update new variables
          UpdateGraphAndTableValuesGivenPlacement(
              new_graph, new_available_nodes, module_placement,
              new_processed_nodes, new_data_tables, new_next_run_blocked_nodes,
              blocked_nodes);

          // Go to a new decision branch
          PlaceNodesRecursively(
              std::move(new_available_nodes), std::move(new_processed_nodes),
              std::move(new_graph), std::move(new_current_run), current_plan,
              std::move(new_data_tables), blocked_nodes,
              std::move(new_next_run_blocked_nodes), new_streamed_data_size);
        }
        available_module_placements.clear();
      }
      // Check early run finishing option
      if (!reduce_single_runs_ && !current_run.empty() &&
          !(use_max_runs_cap_ && current_plan.size() + 1 > min_runs_)) {
        auto new_current_plan = current_plan;
        new_current_plan.push_back(current_run);
        auto new_blocked_nodes = blocked_nodes;
        for (const auto& blocked_node : next_run_blocked_nodes) {
          new_blocked_nodes.insert(blocked_node);
        }
        PlaceNodesRecursively(available_nodes, processed_nodes, graph, {},
                              new_current_plan, data_tables, new_blocked_nodes,
                              {}, streamed_data_size);
      }
      // Update the values for this decision branch.
      current_run.insert(current_run.begin() + current_placement.first,
                         current_placement.second);
      streamed_data_size += GetNewStreamedDataSize(
          current_run, current_placement.second.node_name, data_tables, graph);
      UpdateGraphAndTableValuesGivenPlacement(
          graph, available_nodes, current_placement.second, processed_nodes,
          data_tables, next_run_blocked_nodes, blocked_nodes);
    }
  }
  if (!current_run.empty()) {
    current_plan.push_back(std::move(current_run));
  }
  AddPlanToAllPlansAndMeasureTime(current_plan, processed_nodes, data_tables,
                                  streamed_data_size);
}

auto ElasticSchedulingGraphParser::GetTimeoutStatus() const -> bool {
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

void ElasticSchedulingGraphParser::SetTimeLimit(
    const std::chrono::system_clock::time_point new_time_limit) {
  time_limit_ = new_time_limit;
  trigger_timeout_ = false;
  min_runs_ = std::numeric_limits<int>::max();
  resulting_plan_.clear();
  statistics_counters_ = {0, 0};
}
