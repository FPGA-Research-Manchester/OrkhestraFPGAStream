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
#include <chrono>
#include <limits>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "accelerator_library_interface.hpp"
#include "module_selection.hpp"
#include "pr_module_data.hpp"
#include "pre_scheduling_processor.hpp"
#include "scheduled_module.hpp"
#include "scheduling_data.hpp"

using orkhestrafs::core_interfaces::hw_library::OperationPRModules;
using orkhestrafs::dbmstodspi::AcceleratorLibraryInterface;
using orkhestrafs::dbmstodspi::ModuleSelection;
using orkhestrafs::dbmstodspi::PairHash;
using orkhestrafs::dbmstodspi::PreSchedulingProcessor;
using orkhestrafs::dbmstodspi::ScheduledModule;
using orkhestrafs::dbmstodspi::scheduling_data::ExecutionPlanSchedulingData;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to preprocess given nodes before scheduling.
 */
class ElasticSchedulingGraphParser {
 public:
  ElasticSchedulingGraphParser(
      const std::map<QueryOperationType, OperationPRModules>& hw_library,
      std::pair<std::vector<std::vector<ModuleSelection>>,
                std::vector<std::vector<ModuleSelection>>>
          heuristics,
      std::unordered_set<std::string> constrained_first_nodes,
      AcceleratorLibraryInterface& drivers, const bool use_max_runs_cap,
      const bool reduce_single_runs, const bool prioritise_children)
      : hw_library_{hw_library},
        heuristics_{std::move(heuristics)},
        statistics_counters_{0, 0},
        constrained_first_nodes_{std::move(constrained_first_nodes)},
        drivers_{drivers},
        time_limit_{std::chrono::system_clock::time_point::max()},
        trigger_timeout_{false},
        use_max_runs_cap_{use_max_runs_cap},
        reduce_single_runs_{reduce_single_runs},
        prioritise_children_{prioritise_children},

        min_runs_{std::numeric_limits<int>::max()},
        pre_scheduler_{hw_library, drivers} {};

  void PreprocessNodes(
      std::unordered_set<std::string>& available_nodes,
      std::unordered_set<std::string>& processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::map<std::string, TableMetadata>& data_tables);

  void PlaceNodesRecursively(
      std::unordered_set<std::string> available_nodes,
      std::unordered_set<std::string> processed_nodes,
      std::unordered_map<std::string, SchedulingQueryNode> graph,
      std::vector<ScheduledModule> current_run,
      std::vector<std::vector<ScheduledModule>> current_plan,
      std::map<std::string, TableMetadata> data_tables,
      std::unordered_set<std::string> blocked_nodes,
      std::unordered_set<std::string> next_run_blocked_nodes,
      int streamed_data_size);

  [[nodiscard]] auto GetTimeoutStatus() const -> bool;
  auto GetResultingPlan() -> std::map<std::vector<std::vector<ScheduledModule>>,
                                      ExecutionPlanSchedulingData>;
  auto GetStats() -> std::pair<int, int>;

  void SetTimeLimit(std::chrono::system_clock::time_point new_time_limit);

 private:
  int min_runs_;
  const std::map<QueryOperationType, OperationPRModules> hw_library_;
  const std::pair<std::vector<std::vector<ModuleSelection>>,
                  std::vector<std::vector<ModuleSelection>>>
      heuristics_;
  std::pair<int, int> statistics_counters_;
  const std::unordered_set<std::string> constrained_first_nodes_;
  AcceleratorLibraryInterface& drivers_;
  std::chrono::system_clock::time_point time_limit_;
  bool trigger_timeout_;
  const bool use_max_runs_cap_;
  std::map<std::vector<std::vector<ScheduledModule>>,
           ExecutionPlanSchedulingData>
      resulting_plan_;
  const bool reduce_single_runs_;
  const bool prioritise_children_;
  PreSchedulingProcessor pre_scheduler_;

  /*struct CustomCmp {
    bool operator()(const std::tuple<std::unordered_set<std::string>,
                                     std::unordered_set<std::string>,
  std::vector<ScheduledModule>>& a, const
  std::tuple<std::unordered_set<std::string>, std::unordered_set<std::string>,
  std::vector<ScheduledModule>>& b) const { std::string combined_string_a; for
  (const auto& available :std::get<0>(a)){ combined_string_a+=available;
      }
      for (const auto& blocked :std::get<1>(a)){
        combined_string_a+=blocked;
      }
      for (const auto& module :std::get<2>(a)){
        combined_string_a+=module.bitstream;
      }

      std::string combined_string_b;
      for (const auto& available :std::get<0>(b)){
        combined_string_b+=available;
      }
      for (const auto& blocked :std::get<1>(b)){
        combined_string_b+=blocked;
      }
      for (const auto& module :std::get<2>(b)){
        combined_string_b+=module.bitstream;
      }

      return combined_string_a.length() < combined_string_b.length();
    }
  };
  std::map<
      std::tuple<std::unordered_set<std::string>,
                 std::unordered_set<std::string>, std::vector<ScheduledModule>>,
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>, CustomCmp>
      saved_placements_;*/
  /*std::unordered_map<
      std::string,
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>>
      saved_placements_;*/
  std::unordered_map<
      std::string,
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>>
      saved_placements_;

  void AddPlanToAllPlansAndMeasureTime(
      const std::vector<std::vector<ScheduledModule>>& current_plan,
      const std::unordered_set<std::string>& processed_nodes,
      const std::map<std::string, TableMetadata>& data_tables,
      int streamed_data_size);

  void GetAllAvailableModulePlacementsInCurrentRun(
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          available_module_placements,
      const std::unordered_set<std::string>& available_nodes,
      const std::vector<ScheduledModule>& current_run,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      const std::unordered_set<std::string>& blocked_nodes,
      const std::map<std::string, TableMetadata>& data_tables);

  static auto IsSubsetOf(const std::unordered_set<std::string>& a,
                         const std::unordered_set<std::string>& b) -> bool;

  static auto GetNewStreamedDataSize(
      const std::vector<ScheduledModule>& current_run,
      const std::string& node_name,
      const std::map<std::string, TableMetadata>& data_tables,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph) -> int;

  static auto IsTableEqualForGivenNode(
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      const std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      const std::string& next_node_name,
      const std::map<std::string, TableMetadata>& data_tables,
      const std::map<std::string, TableMetadata>& new_tables) -> bool;

  static void UpdateNextNodeTables(
      const std::string& node_name,
      std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      const std::vector<std::string>& resulting_tables);

  auto GetResultingTables(const std::vector<std::string>& table_names,
                          const std::map<std::string, TableMetadata>& tables,
                          QueryOperationType operation)
      -> std::vector<std::string>;

  static void UpdateGraphCapacities(
      const std::vector<int>& missing_utility,
      std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      const std::string& node_name, bool is_node_fully_processed);

  static auto FindMissingUtility(const std::vector<int>& bitstream_capacity,
                                 std::vector<int>& missing_capacity,
                                 const std::vector<int>& node_capacity,
                                 QueryOperationType operation_type,
                                 bool is_composed) -> bool;

  auto CheckForSkippableSortOperations(
      const std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      const std::map<std::string, TableMetadata>& new_tables,
      const std::string& node_name) -> std::vector<std::string>;

  static auto GetModuleIndex(
      int start_location_index,
      const std::vector<std::pair<int, int>>& taken_positions) -> int;

  static void ReduceSelectionAccordingToHeuristics(
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          resulting_module_placements,
      const std::vector<std::vector<ModuleSelection>>& heuristics);

  auto GetBitstreamEndFromLibrary(int chosen_bitstream_index,
                                  int chosen_column_position,
                                  QueryOperationType current_operation)
      -> std::pair<std::string, int>;

  auto FindAllAvailableBitstreamsAfterMinPos(
      QueryOperationType current_operation, int min_position,
      const std::vector<std::pair<int, int>>& taken_positions,
      const std::vector<std::vector<std::string>>& bitstream_start_locations)
      -> std::vector<std::tuple<int, int, int>>;

  auto GetChosenModulePlacements(
      const std::string& node_name, QueryOperationType current_operation,
      const std::vector<std::vector<ModuleSelection>>& heuristics,
      int min_position, const std::vector<std::pair<int, int>>& taken_positions,
      const std::vector<std::vector<std::string>>& bitstream_start_locations,
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          module_placements,
      bool is_composed) -> bool;

  auto CurrentRunHasFirstModule(const std::vector<ScheduledModule>& current_run,
                                const std::string& node_name,
                                const QueryOperationType operation_type)
      -> bool;

  auto RemoveUnavailableNodesInThisRun(
      const std::unordered_set<std::string>& available_nodes,
      const std::vector<ScheduledModule>& current_run,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      const std::unordered_set<std::string>& blocked_nodes,
      const std::map<std::string, TableMetadata>& data_tables)
      -> std::unordered_set<std::string>;

  static auto GetMinPositionInCurrentRun(
      const std::vector<ScheduledModule>& current_run,
      const std::string& node_name,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph) -> int;

  static auto GetTakenColumns(const std::vector<ScheduledModule>& current_run)
      -> std::vector<std::pair<int, int>>;

  void GetScheduledModulesForNodeAfterPosOrig(
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      int min_position, const std::string& node_name,
      const std::vector<std::pair<int, int>>& taken_positions,
      const std::map<std::string, TableMetadata>& data_tables,
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          module_placements,
      bool is_composed);
  void GetScheduledModulesForNodeAfterPos(
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      const std::vector<ScheduledModule>& current_run,
      const std::string& node_name,
      const std::map<std::string, TableMetadata>& data_tables,
      std::unordered_set<std::pair<int, ScheduledModule>, PairHash>&
          module_placements);

  void UpdateGraphAndTableValuesGivenPlacement(
      std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      std::unordered_set<std::string>& new_available_nodes,
      const ScheduledModule& module_placement,
      std::unordered_set<std::string>& new_processed_nodes,
      std::map<std::string, TableMetadata>& new_data_tables,
      std::unordered_set<std::string>& new_next_run_blocked_nodes,
      const std::unordered_set<std::string>& blocked_nodes);

  auto UpdateGraphCapacitiesAndTables(
      std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      std::map<std::string, TableMetadata>& new_data_tables,
      const std::string& bitstream, const std::string& node_name,
      const std::vector<int>& capacity, QueryOperationType operation,
      bool is_composed) -> bool;

  static auto CreateNewAvailableNodes(
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::unordered_set<std::string>& available_nodes,
      std::unordered_set<std::string>& processed_nodes,
      const std::string& node_name);

  void UpdateAvailableNodesAndSatisfyingBitstreamsList(
      const std::string& node_name,
      std::unordered_map<std::string, SchedulingQueryNode>& new_graph,
      std::unordered_set<std::string>& new_available_nodes,
      std::map<std::string, TableMetadata>& new_tables,
      std::unordered_set<std::string>& new_processed_nodes,
      QueryOperationType operation, bool satisfied_requirements,
      std::unordered_set<std::string>& next_run_blocked_nodes,
      const std::unordered_set<std::string>& blocked_nodes);

  void GetNewBlockedNodes(
      std::unordered_set<std::string>& next_run_blocked_nodes,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      QueryOperationType operation, const std::string& node_name,
      const std::unordered_set<std::string>& blocked_nodes);

  void FindDataSensitiveNodeNames(
      const std::string& node_name,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::unordered_set<std::string>& new_next_run_blocked_nodes,
      const std::unordered_set<std::string>& blocked_nodes);
};

}  // namespace orkhestrafs::dbmstodspi