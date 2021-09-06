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
#include "query_manager_interface.hpp"

namespace easydspi::dbmstodspi {

class QueryManager : public QueryManagerInterface {
 public:
  ~QueryManager() override = default;
  auto GetCurrentLinks(
      const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          all_reuse_links)
      -> std::map<std::string, std::map<int, MemoryReuseTargets>> override;
  auto CreateFPGAManager(MemoryManagerInterface* memory_manager)
      -> std::unique_ptr<FPGAManagerInterface> override;
  auto SetupAccelerationNodesForExecution(
      DataManagerInterface* data_manager,
      MemoryManagerInterface* memory_manager,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>*
          input_memory_blocks,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>*
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>* input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>*
          output_stream_sizes,
      const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes)
      -> std::pair<
          std::vector<AcceleratedQueryNode>,
          std::map<std::string, std::vector<StreamResultParameters>>> override;
  void LoadNextBitstreamIfNew(
      MemoryManagerInterface* memory_manager,
      std::string bitstream_file_name, Config config) override;
  auto ScheduleUnscheduledNodes(
      std::vector<std::shared_ptr<QueryNode>> unscheduled_root_nodes,
      Config config)
      -> std::pair<
          std::map<std::string, std::map<int, MemoryReuseTargets>>,
          std::queue<std::pair<ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>> override;
  auto IsRunValid(std::vector<AcceleratedQueryNode> current_run)
      -> bool override;
  void ExecuteAndProcessResults() override;
 private:

};

}  // namespace easydspi::dbmstodspi