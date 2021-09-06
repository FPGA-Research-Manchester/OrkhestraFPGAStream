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
#include <string>
#include <utility>

#include "accelerated_query_node.hpp"
#include "config.hpp"
#include "data_manager_interface.hpp"
#include "fpga_manager_interface.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
#include "query_scheduling_data.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using easydspi::core_interfaces::query_scheduling_data::MemoryReuseTargets;
using easydspi::core_interfaces::query_scheduling_data::QueryNode;
using easydspi::core_interfaces::query_scheduling_data::RecordSizeAndCount;
using easydspi::core_interfaces::query_scheduling_data::StreamResultParameters;
using easydspi::dbmstodspi::AcceleratedQueryNode;
using easydspi::dbmstodspi::FPGAManagerInterface;
using easydspi::dbmstodspi::MemoryBlockInterface;

namespace easydspi::dbmstodspi {

class QueryManagerInterface {
 public:
  virtual ~QueryManagerInterface() = default;
  virtual auto GetCurrentLinks(
      const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          all_reuse_links)
      -> std::map<std::string, std::map<int, MemoryReuseTargets>> = 0;
  virtual auto CreateFPGAManager(MemoryManagerInterface* memory_manager)
      -> std::unique_ptr<FPGAManagerInterface> = 0;
  virtual auto SetupAccelerationNodesForExecution(
      DataManagerInterface* data_manager,
      MemoryManagerInterface* memory_manager,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>*
          input_memory_blocks,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>*
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>*
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>*
          output_stream_sizes,
      const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes)
      -> std::pair<
          std::vector<AcceleratedQueryNode>,
          std::map<std::string, std::vector<StreamResultParameters>>> = 0;
  virtual void LoadNextBitstreamIfNew(MemoryManagerInterface* memory_manager,
                                      std::string bitstream_file_name,
                                      Config config) = 0;
  virtual auto ScheduleUnscheduledNodes(
      std::vector<std::shared_ptr<QueryNode>> unscheduled_root_nodes,
      Config config)
      -> std::pair<
          std::map<std::string, std::map<int, MemoryReuseTargets>>,
          std::queue<std::pair<ConfigurableModulesVector,
                               std::vector<std::shared_ptr<QueryNode>>>>> = 0;
  virtual auto IsRunValid(std::vector<AcceleratedQueryNode> current_run)
      -> bool = 0;
  virtual void ExecuteAndProcessResults() = 0;
};

}  // namespace easydspi::dbmstodspi