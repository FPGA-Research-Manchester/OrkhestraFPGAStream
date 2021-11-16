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

namespace orkhestrafs::dbmstodspi {

class QueryManager : public QueryManagerInterface {
 public:
  ~QueryManager() override = default;
  auto GetCurrentLinks(
      const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          all_reuse_links)
      -> std::map<std::string, std::map<int, MemoryReuseTargets>> override;
  auto SetupAccelerationNodesForExecution(
      DataManagerInterface* data_manager,
      MemoryManagerInterface* memory_manager,
      AcceleratorLibraryInterface* accelerator_library,
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
  void LoadNextBitstreamIfNew(MemoryManagerInterface* memory_manager,
                              std::string bitstream_file_name,
                              Config config) override;
  auto ScheduleUnscheduledNodes(
      std::vector<std::shared_ptr<QueryNode>> unscheduled_root_nodes,
      Config config, NodeSchedulerInterface& node_scheduler)
      -> std::pair<std::map<std::string, std::map<int, MemoryReuseTargets>>,
                   std::queue<std::pair<
                       ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>> override;
  auto IsRunValid(std::vector<AcceleratedQueryNode> current_run)
      -> bool override;
  void ExecuteAndProcessResults(
      FPGAManagerInterface* fpga_manager,
      const DataManagerInterface* data_manager,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<AcceleratedQueryNode>& execution_query_nodes) override;
  void FreeMemoryBlocks(
      MemoryManagerInterface* memory_manager,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          reuse_links,
      const std::vector<std::string>& scheduled_node_names) override;
  auto ScheduleNextSetOfNodes(
      std::vector<std::shared_ptr<QueryNode>> query_nodes,
      const std::vector<std::string>& first_node_names,
      std::vector<std::string>& starting_nodes,
      std::vector<std::string>& processed_nodes,
      std::map<std::string, SchedulingQueryNode>& graph,
      std::map<std::string, TableMetadata>& tables,
      AcceleratorLibraryInterface& drivers, Config config,
      NodeSchedulerInterface& node_scheduler)
      -> std::queue<std::pair<ConfigurableModulesVector,
                             std::vector<std::shared_ptr<QueryNode>>>> override;

 private:
  static void CheckTableData(const DataManagerInterface* data_manager,
                             const TableData& expected_table,
                             const TableData& resulting_table);
  static void CheckResults(
      const DataManagerInterface* data_manager,
      const std::unique_ptr<MemoryBlockInterface>& memory_device, int row_count,
      const std::string& filename,
      const std::vector<std::vector<int>>& node_parameters, int stream_index);
  static void WriteResults(
      const DataManagerInterface* data_manager,
      const std::unique_ptr<MemoryBlockInterface>& memory_device, int row_count,
      const std::string& filename,
      const std::vector<std::vector<int>>& node_parameters, int stream_index);
  static void CopyMemoryData(
      int table_size,
      const std::unique_ptr<MemoryBlockInterface>& source_memory_device,
      const std::unique_ptr<MemoryBlockInterface>& target_memory_device);
  void ProcessResults(
      const DataManagerInterface* data_manager,
      std::array<int, query_acceleration_constants::kMaxIOStreamCount>
          result_sizes,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::map<std::string,
                     std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          allocated_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes);
  static void StoreStreamResultParameters(
      std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<int>& stream_ids, const QueryNode& node,
      const std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks);
  static auto CreateStreamParams(
      bool is_input, const QueryNode& node,
      AcceleratorLibraryInterface* accelerator_library,
      const std::vector<int>& stream_ids,
      const std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks,
      const std::vector<RecordSizeAndCount>& stream_sizes)
      -> std::vector<StreamDataParameters>;
  static auto GetRecordSizeFromParameters(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& node_parameters, int stream_index)
      -> int;
  static void AllocateOutputMemoryBlocks(
      MemoryManagerInterface* memory_manager,
      const DataManagerInterface* data_manager,
      std::vector<std::unique_ptr<MemoryBlockInterface>>& output_memory_blocks,
      const QueryNode& node,
      std::vector<RecordSizeAndCount>& output_stream_sizes);
  static void AllocateInputMemoryBlocks(
      MemoryManagerInterface* memory_manager,
      const DataManagerInterface* data_manager,
      std::vector<std::unique_ptr<MemoryBlockInterface>>& input_memory_blocks,
      const QueryNode& node,
      const std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      std::vector<RecordSizeAndCount>& input_stream_sizes);
  static void InitialiseStreamSizeVector(
      std::map<std::string, std::vector<RecordSizeAndCount>>& stream_sizes,
      int stream_count, const std::string& node_name);
  static void InitialiseMemoryBlockVector(
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          memory_blocks,
      int stream_count, const std::string& node_name);
  static void InitialiseVectorSizes(
      const std::vector<std::shared_ptr<QueryNode>>& scheduled_nodes,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes);
  void AddQueryNodes(std::vector<AcceleratedQueryNode>& query_nodes_vector,
                     std::vector<StreamDataParameters>&& input_params,
                     std::vector<StreamDataParameters>&& output_params,
                     const QueryNode& node);
};

}  // namespace orkhestrafs::dbmstodspi