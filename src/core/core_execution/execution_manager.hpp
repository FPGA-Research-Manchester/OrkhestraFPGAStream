﻿/*
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
#include <utility>

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
#include "state_interface.hpp"

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
using orkhestrafs::dbmstodspi::StateInterface;

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
                   std::unique_ptr<NodeSchedulerInterface> scheduler)
      : current_state_{std::move(start_state)},
        data_manager_{std::move(data_manager)},
        memory_manager_{std::move(memory_manager)},
        query_manager_{std::move(query_manager)},
        scheduler_{std::move(scheduler)},
        config_{std::move(config)},
        accelerator_library_{std::move(
            driver_factory->CreateAcceleratorLibrary(memory_manager_.get()))},
        fpga_manager_{std::move(
            driver_factory->CreateFPGAManager(accelerator_library_.get()))} {};

  void SetFinishedFlag() override;

  void Execute(
      std::unique_ptr<ExecutionPlanGraphInterface> execution_graph) override;

  auto IsUnscheduledNodesGraphEmpty() -> bool override;
  void ScheduleUnscheduledNodes() override;
  auto IsARunScheduled() -> bool override;
  void SetupNextRunData() override;
  auto IsRunReadyForExecution() -> bool override;
  auto IsRunValid() -> bool override;
  void ExecuteAndProcessResults() override;

 private:
  // Initial inputs
  std::unique_ptr<QueryManagerInterface> query_manager_;
  std::unique_ptr<DataManagerInterface> data_manager_;
  std::unique_ptr<MemoryManagerInterface> memory_manager_;
  std::unique_ptr<ExecutionPlanGraphInterface> unscheduled_graph_;
  std::unique_ptr<AcceleratorLibraryInterface> accelerator_library_;
  std::unique_ptr<FPGAManagerInterface> fpga_manager_;
  std::unique_ptr<NodeSchedulerInterface> scheduler_;
  const Config config_;
  // State status
  std::unique_ptr<StateInterface> current_state_;
  bool busy_flag_ = false;
  // Variables used throughout different states.
  std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_links_;
  std::map<std::string, std::map<int, MemoryReuseTargets>> current_reuse_links_;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      input_memory_blocks_;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      output_memory_blocks_;
  std::map<std::string, std::vector<RecordSizeAndCount>> input_stream_sizes_;
  std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes_;

  std::queue<std::pair<ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>
      query_node_runs_queue_;

  // Clear for each run
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters_;
  std::vector<AcceleratedQueryNode> query_nodes_;
  std::vector<std::string> scheduled_node_names_;

  auto PopNextScheduledRun() -> std::vector<std::shared_ptr<QueryNode>>;
};
}  // namespace orkhestrafs::core::core_execution