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

#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <chrono>

#include "accelerated_query_node.hpp"
#include "accelerator_library_interface.hpp"
#include "data_manager_interface.hpp"
#include "execution_manager_interface.hpp"
#include "fpga_driver_factory_interface.hpp"
#include "fpga_manager_interface.hpp"
#include "graph_processing_fsm_interface.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
#include "node_scheduler_interface.hpp"
#include "query_manager_interface.hpp"
#include "query_scheduling_data.hpp"
#include "scheduling_query_node.hpp"
#include "state_interface.hpp"
#include "graph_creator_interface.hpp"

using orkhestrafs::core_interfaces::Config;
using orkhestrafs::core_interfaces::ExecutionManagerInterface;
using orkhestrafs::core_interfaces::ExecutionPlanGraphInterface;

using orkhestrafs::core_interfaces::query_scheduling_data::RecordSizeAndCount;
using orkhestrafs::core_interfaces::query_scheduling_data::
    StreamResultParameters;
using orkhestrafs::dbmstodspi::AcceleratedQueryNode;
using orkhestrafs::dbmstodspi::AcceleratorLibraryInterface;
using orkhestrafs::dbmstodspi::DataManagerInterface;
using orkhestrafs::dbmstodspi::FPGADriverFactoryInterface;
using orkhestrafs::dbmstodspi::FPGAManagerInterface;
using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;
using orkhestrafs::dbmstodspi::MemoryManagerInterface;
using orkhestrafs::dbmstodspi::NodeSchedulerInterface;
using orkhestrafs::dbmstodspi::QueryManagerInterface;
using orkhestrafs::dbmstodspi::SchedulingQueryNode;
using orkhestrafs::dbmstodspi::StateInterface;
using orkhestrafs::dbmstodspi::GraphCreatorInterface;

namespace orkhestrafs::core::core_execution {

class ExecutionManager : public ExecutionManagerInterface,
                         public GraphProcessingFSMInterface {
 public:
  ~ExecutionManager() override = default;
  ExecutionManager(Config config,
                   std::unique_ptr<QueryManagerInterface> query_manager,
                   std::unique_ptr<DataManagerInterface> data_manager,
                   std::unique_ptr<MemoryManagerInterface> memory_manager,
                   std::unique_ptr<StateInterface> start_state,
                   std::unique_ptr<FPGADriverFactoryInterface> driver_factory,
                   std::unique_ptr<NodeSchedulerInterface> scheduler,
                   std::unique_ptr<GraphCreatorInterface> graph_creator)
      : current_state_{std::move(start_state)},
        data_manager_{std::move(data_manager)},
        memory_manager_{std::move(memory_manager)},
        query_manager_{std::move(query_manager)},
        scheduler_{std::move(scheduler)},
        graph_creator_{std::move(graph_creator)},
        config_{std::move(config)},
        accelerator_library_{std::move(
            driver_factory->CreateAcceleratorLibrary(memory_manager_.get()))},
        fpga_manager_{std::move(
            driver_factory->CreateFPGAManager(accelerator_library_.get()))} {};

  auto IsSWBackupEnabled() -> bool override;
  auto IsHWPrintEnabled() -> bool override;
  void LoadBitstream(ScheduledModule new_module) override;
  auto GetCurrentHW() -> std::map<QueryOperationType, OperationPRModules> override;
  void SetHWPrint(bool print_hw) override;
  void SetStartTimer() override;
  void PrintExecTime() override;
  void AddNewNodes(std::string graph_filename) override;
  void LoadStaticTables() override;
  void SetInteractive(bool is_interactive) override;
  auto IsInteractive()->bool override;
  void ChangeSchedulingTimeLimit(double new_time_limit) override;
  void ChangeExecutionTimeLimit(int new_time_limit) override;
  void LoadStaticBitstream() override;
  void SetClockSpeed(int new_clock_speed) override;
  void PrintHWState() override;
  auto GetFPGASpeed() -> int override;
  void SetFinishedFlag() override;
  void UpdateAvailableNodesGraph() override;
  void Execute(
      std::unique_ptr<ExecutionPlanGraphInterface> execution_graph) override;

  auto IsUnscheduledNodesGraphEmpty() -> bool override;
  void ScheduleUnscheduledNodes() override;
  auto IsARunScheduled() -> bool override;
  void SetupNextRunData() override;
  auto IsRunReadyForExecution() -> bool override;
  /*auto IsRunValid() -> bool override;*/
  void ExecuteAndProcessResults() override;
  void PrintCurrentStats() override;
  void SetupSchedulingData(bool setup_bitstreams) override;

  void BenchmarkScheduleUnscheduledNodes() override;
  auto IsBenchmarkDone() -> bool override;

 private:
  long config_time_;
  // Initial inputs
  std::unique_ptr<QueryManagerInterface> query_manager_;
  std::unique_ptr<DataManagerInterface> data_manager_;
  std::unique_ptr<MemoryManagerInterface> memory_manager_;
  std::unique_ptr<ExecutionPlanGraphInterface> unscheduled_graph_;
  std::unique_ptr<AcceleratorLibraryInterface> accelerator_library_;
  std::unique_ptr<FPGAManagerInterface> fpga_manager_;
  std::unique_ptr<NodeSchedulerInterface> scheduler_;
  std::unique_ptr<GraphCreatorInterface> graph_creator_;
  Config config_;
  // State status
  bool print_hw_ = false;
  std::chrono::steady_clock::time_point exec_begin;
  bool is_interactive_ = false;
  std::unique_ptr<StateInterface> current_state_;
  bool busy_flag_ = false;
  // New TableMetadata variables.
  std::vector<std::string> current_routing_;

  std::map<std::string, TableMetadata> current_tables_metadata_;
  std::unordered_map<std::string, MemoryBlockInterface*> table_memory_blocks_;
  std::unordered_map<std::string, SchedulingQueryNode> current_query_graph_;
  std::unordered_map<std::string, int> table_counter_;

  std::unordered_set<std::string> current_available_node_names_;
  std::unordered_set<std::string> nodes_constrained_to_first_;
  std::unordered_set<std::string> processed_nodes_;
  std::unordered_set<std::string> blocked_nodes_;

  std::vector<QueryNode*> current_available_node_pointers_;

  std::queue<std::pair<std::vector<ScheduledModule>, std::vector<QueryNode*>>>
      query_node_runs_queue_;
  std::vector<ScheduledModule> current_configuration_;

  // Clear for each run
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters_;
  std::vector<AcceleratedQueryNode> query_nodes_;
  std::vector<std::string> scheduled_node_names_;

  auto GetModuleCapacity(int module_position, QueryOperationType operation)
      -> std::vector<int>;
  auto PopNextScheduledRun() -> std::vector<QueryNode*>;

  // TODO(Kaspar): Move this to a different class
  static auto GetCurrentNodeIndexFromNextNode(QueryNode* current_node,
                                              QueryNode* next_node) -> int;
  static void InitialiseTables(
      std::map<std::string, TableMetadata>& tables_metadata,
      std::vector<QueryNode*> current_available_node_pointers,
      const QueryManagerInterface* query_manager,
      const DataManagerInterface* data_manager, bool is_benchmark);
  static void SetupSchedulingGraphAndConstrainedNodes(
      const std::vector<QueryNode*>& all_query_nodes,
      std::unordered_map<std::string, SchedulingQueryNode>&
          current_scheduling_graph,
      AcceleratorLibraryInterface& hw_library,
      std::unordered_set<std::string>& constrained_nodes_vector,
      const std::map<std::string, TableMetadata>& tables_metadata);
  static void RemoveUnusedTables(
      std::map<std::string, TableMetadata>& tables_metadata,
      const std::vector<QueryNode*>& all_nodes,const std::vector<std::string> frozen_tables);
  static void SetupTableDependencies(
      const std::vector<QueryNode*>& all_nodes,
      std::unordered_set<std::string>& blocked_nodes,
      std::unordered_map<std::string, int>& table_counter);

  static void AddSchedulingNodeToGraph(
      QueryNode* const& node,
      std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph,
      AcceleratorLibraryInterface& accelerator_library);
  static void AddSavedNodesToConstrainedList(
      QueryNode* const& node,
      std::unordered_set<std::string>& constrained_nodes);
  static void AddFirstModuleNodesToConstrainedList(
      QueryNode* const& node,
      std::unordered_set<std::string>& constrained_nodes,
      AcceleratorLibraryInterface& accelerator_library);
  static void AddSplittingNodesToConstrainedList(
      std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph,
      std::unordered_set<std::string>& constrained_nodes);
};
}  // namespace orkhestrafs::core::core_execution