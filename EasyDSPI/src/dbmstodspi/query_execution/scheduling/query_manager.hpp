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
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
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
  void QueryManager::StoreStreamResultPrameters(
      std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<int>& stream_ids, const QueryNode& node,
      const std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks);
  auto QueryManager::CreateStreamParams(
      const std::vector<int>& stream_ids,
      const std::vector<std::vector<int>>& node_parameters,
      const std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks,
      const std::vector<RecordSizeAndCount>& stream_sizes)
      -> std::vector<StreamDataParameters>;
  auto QueryManager::GetRecordSizeFromParameters(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& node_parameters, int stream_index)
      -> int;
  void QueryManager::AllocateOutputMemoryBlocks(
      MemoryManagerInterface* memory_manager,
      const DataManagerInterface* data_manager,
      std::vector<std::unique_ptr<MemoryBlockInterface>>& output_memory_blocks,
      const QueryNode& node,
      std::vector<RecordSizeAndCount>& output_stream_sizes);
  void QueryManager::AllocateInputMemoryBlocks(
      MemoryManagerInterface* memory_manager,
      const DataManagerInterface* data_manager,
      std::vector<std::unique_ptr<MemoryBlockInterface>>& input_memory_blocks,
      const QueryNode& node,
      const std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      std::vector<RecordSizeAndCount>& input_stream_sizes);
  void QueryManager::InitialiseStreamSizeVector(
      std::map<std::string, std::vector<RecordSizeAndCount>>& stream_sizes,
      int stream_count, const std::string& node_name);
  void QueryManager::InitialiseMemoryBlockVector(
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          memory_blocks,
      int stream_count, const std::string& node_name);
  void QueryManager::InitialiseVectorSizes(
      const std::vector<std::shared_ptr<QueryNode>>& scheduled_nodes,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes);
};

}  // namespace easydspi::dbmstodspi