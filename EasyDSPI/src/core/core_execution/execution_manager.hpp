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

#include <string>

#include "accelerated_query_node.hpp"
#include "data_manager_interface.hpp"
#include "execution_manager_interface.hpp"
#include "fpga_manager_interface.hpp"
#include "graph_processing_fsm_interface.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
#include "query_scheduling_data.hpp"
#include "state_interface.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionManagerInterface;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::core_interfaces::query_scheduling_data::MemoryReuseTargets;
using easydspi::core_interfaces::query_scheduling_data::RecordSizeAndCount;
using easydspi::core_interfaces::query_scheduling_data::StreamResultParameters;
using easydspi::dbmstodspi::AcceleratedQueryNode;
using easydspi::dbmstodspi::DataManagerInterface;
using easydspi::dbmstodspi::FPGAManagerInterface;
using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::MemoryManagerInterface;
using easydspi::dbmstodspi::StateInterface;

namespace easydspi::core::core_execution {

class ExecutionManager : public ExecutionManagerInterface,
                         public GraphProcessingFSMInterface {
 public:
  ~ExecutionManager() override = default;
  ExecutionManager(std::unique_ptr<DataManagerInterface> data_manager,
                   std::unique_ptr<MemoryManagerInterface> memory_manager,
                   std::unique_ptr<StateInterface> start_state)
      : current_state_{std::move(start_state)},
        data_manager_{std::move(data_manager)},
        memory_manager_{std::move(memory_manager)} {}

  void setFinishedFlag() override;

  void execute(std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
                   execution_input) override;
  std::string getCurrentGraphData() override;
  void setCurrentGraphData(std::string new_data) override;
  const Config& getCurrentConfig() override;
  void setCurrentConfig(const Config& new_config) override;
  const ExecutionPlanGraphInterface* getInitialGraph() override;
  const Config& getInitialConfig() override;

 private:
  // Initial inputs
  std::unique_ptr<DataManagerInterface> data_manager_;
  std::unique_ptr<MemoryManagerInterface> memory_manager_;
  std::unique_ptr<ExecutionPlanGraphInterface> initial_graph_;
  Config initial_config_;
  // State status
  std::unique_ptr<StateInterface> current_state_;
  bool busy_flag_ = false;
  // Variables used throughout different states.
  std::unique_ptr<FPGAManagerInterface> fpga_manager_;
  std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_links;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      input_memory_blocks;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      output_memory_blocks;
  std::map<std::string, std::vector<RecordSizeAndCount>> input_stream_sizes;
  std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes;
  // Clear for each run
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters;
  std::vector<AcceleratedQueryNode> query_nodes;
  std::map<std::string, std::vector<int>> output_ids;
  std::map<std::string, std::vector<int>> input_ids;
  // Remove
  std::string current_graph_data_;
  Config current_config_;
};
}  // namespace easydspi::core::core_execution