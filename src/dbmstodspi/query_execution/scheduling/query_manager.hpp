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
#include "json_reader_interface.hpp"
#include "query_manager_interface.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::JSONReaderInterface;
using orkhestrafs::core_interfaces::query_scheduling_data::NodeRunData;

namespace orkhestrafs::dbmstodspi {

class QueryManager : public QueryManagerInterface {
 public:
  long data_count = 0;
  long merge_count = 0;
  long initialisation_sum = 0;
  void MeasureBitstreamConfigurationSpeed(
      const std::map<QueryOperationType, OperationPRModules>& hw_library,
      MemoryManagerInterface* memory_manager) override;
  explicit QueryManager(std::unique_ptr<JSONReaderInterface> json_reader)
      : json_reader_{std::move(json_reader)} {};
  ~QueryManager() override = default;
  auto GetCurrentLinks(
      std::queue<std::map<std::string, std::map<int, MemoryReuseTargets>>>&
          all_reuse_links)
      -> std::map<std::string, std::map<int, MemoryReuseTargets>> override;
  auto SetupAccelerationNodesForExecution(
      DataManagerInterface* data_manager,
      MemoryManagerInterface* memory_manager,
      AcceleratorLibraryInterface* accelerator_library,
      const std::vector<QueryNode*>& current_query_nodes,
      const std::map<std::string, TableMetadata>& current_tables_metadata,
      std::unordered_map<std::string, MemoryBlockInterface*>&
          table_memory_blocks,
      std::unordered_map<std::string, int>& table_counter)
      -> std::pair<
          std::vector<AcceleratedQueryNode>,
          std::map<std::string, std::vector<StreamResultParameters>>> override;
  void LoadNextBitstreamIfNew(MemoryManagerInterface* memory_manager,
                              std::string bitstream_file_name,
                              Config config) override;
  // auto IsRunValid(std::vector<AcceleratedQueryNode> current_run)
  //    -> bool override;
  void ExecuteAndProcessResults(
      MemoryManagerInterface* memory_manager,
      FPGAManagerInterface* fpga_manager,
      const DataManagerInterface* data_manager,
      std::unordered_map<std::string, MemoryBlockInterface*>&
          table_memory_blocks,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<AcceleratedQueryNode>& execution_query_nodes,
      std::map<std::string, TableMetadata>& scheduling_table_data,
      std::unordered_map<std::string, int>& table_counter) override;
  auto ScheduleNextSetOfNodes(
      std::vector<QueryNode*>& query_nodes,
      const std::unordered_set<std::string>& first_node_names,
      const std::unordered_set<std::string>& starting_nodes,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::map<std::string, TableMetadata>& tables,
      AcceleratorLibraryInterface& drivers, const Config& config,
      NodeSchedulerInterface& node_scheduler,
      const std::vector<ScheduledModule>& current_configuration,
      std::unordered_set<std::string>& skipped_nodes,
      std::unordered_map<std::string, int>& table_counter)
      -> std::queue<std::pair<std::vector<ScheduledModule>,
                              std::vector<QueryNode*>>> override;

  void BenchmarkScheduling(
      const std::unordered_set<std::string>& first_node_names,
      const std::unordered_set<std::string>& starting_nodes,
      std::unordered_set<std::string>& processed_nodes,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::map<std::string, TableMetadata>& tables,
      AcceleratorLibraryInterface& drivers, const Config& config,
      NodeSchedulerInterface& node_scheduler,
      std::vector<ScheduledModule>& current_configuration) override;

  void LoadInitialStaticBitstream(
      MemoryManagerInterface* memory_manager) override;

  void LoadEmptyRoutingPRRegion(
      MemoryManagerInterface* memory_manager,
      AcceleratorLibraryInterface& driver_library) override;

  void LoadPRBitstreams(MemoryManagerInterface* memory_manager,
                        const std::vector<std::string>& bitstream_names,
                        AcceleratorLibraryInterface& driver_library) override;

  auto GetPRBitstreamsToLoadWithPassthroughModules(
      std::vector<ScheduledModule>& current_config,
      const std::vector<ScheduledModule>& next_config, int column_count)
      -> std::pair<std::vector<std::string>,
                   std::vector<std::pair<QueryOperationType, bool>>> override;
  void PrintBenchmarkStats() override;

  // TODO(Kaspar): Put somewhere else!
  auto GetRecordSizeFromParameters(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& node_parameters,
      int stream_index) const -> int override;

 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;
  std::map<std::string, double> benchmark_stats_ = {
      {"pre_process_time", 0}, {"schedule_time", 0},
      {"timeout", 0},          {"cost_eval_time", 0},
      {"overall_time", 0},     {"run_count", 0},
      {"data_amount", 0},      {"configuration_amount", 0},
      {"schedule_count", 0},   {"plan_count", 0},
      {"placed_nodes", 0},     {"discarded_placements", 0}};

  static void CheckTableData(const DataManagerInterface* data_manager,
                             const TableData& expected_table,
                             const TableData& resulting_table);
  static void CheckResults(const DataManagerInterface* data_manager,
                           MemoryBlockInterface* memory_device, int row_count,
                           const std::string& filename,
                           const std::vector<std::vector<int>>& node_parameters,
                           int stream_index);
  static void WriteResults(const DataManagerInterface* data_manager,
                           MemoryBlockInterface* memory_device, int row_count,
                           const std::string& filename,
                           const std::vector<std::vector<int>>& node_parameters,
                           int stream_index);
  static void CopyMemoryData(int table_size,
                             MemoryBlockInterface* source_memory_device,
                             MemoryBlockInterface* target_memory_device);
  static void ProcessResults(
      const DataManagerInterface* data_manager,
      std::array<int, query_acceleration_constants::kMaxIOStreamCount>
          result_sizes,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::unordered_map<std::string, MemoryBlockInterface*>&
          table_memory_blocks,
      std::map<std::string, TableMetadata>& scheduling_table_data);
  static void StoreStreamResultParameters(
      std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<int>& stream_ids, const QueryNode* node,
      const NodeRunData& run_data);
  auto CreateStreamParams(
      bool is_input, const QueryNode* node, const std::vector<int>& stream_ids,
      const NodeRunData& run_data,
      const std::map<std::string, TableMetadata>& current_tables_metadata,
      const std::unordered_map<std::string, MemoryBlockInterface*>&
          table_memory_blocks) -> std::vector<StreamDataParameters>;
  static void AllocateOutputMemoryBlocks(
      MemoryManagerInterface* memory_manager, const NodeRunData& run_data,
      std::unordered_map<std::string, MemoryBlockInterface*>&
          table_memory_blocks);
  static void AllocateInputMemoryBlocks(
      MemoryManagerInterface* memory_manager,
      const DataManagerInterface* data_manager, const NodeRunData& run_data,
      const std::vector<std::vector<int>>& input_stream_parameters,
      const std::map<std::string, TableMetadata>& current_tables_metadata,
      std::unordered_map<std::string, MemoryBlockInterface*>&
          table_memory_blocks);
  static void InitialiseStreamSizeVector(
      std::map<std::string, std::vector<RecordSizeAndCount>>& stream_sizes,
      int stream_count, const std::string& node_name);
  static void InitialiseMemoryBlockVector(
      std::map<std::string, std::vector<MemoryBlockInterface*>>& memory_blocks,
      int stream_count, const std::string& node_name);
  static void InitialiseVectorSizes(
      const std::vector<std::shared_ptr<QueryNode>>& scheduled_nodes,
      std::map<std::string, std::vector<MemoryBlockInterface*>>&
          input_memory_blocks,
      std::map<std::string, std::vector<MemoryBlockInterface*>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes);
  static void AddQueryNodes(
      std::vector<AcceleratedQueryNode>& query_nodes_vector,
      std::vector<StreamDataParameters>&& input_params,
      std::vector<StreamDataParameters>&& output_params, QueryNode* node,
      const NodeRunData& run_data);
  static void UpdateTableData(
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      std::map<std::string, TableMetadata>& scheduling_table_data,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          reuse_links,
      std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph);
  static void CropSortedStatus(
      std::map<std::string, TableMetadata>& scheduling_table_data,
      const std::string& filename);
};

}  // namespace orkhestrafs::dbmstodspi