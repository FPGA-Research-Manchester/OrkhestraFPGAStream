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

#include "gmock/gmock.h"
#include "query_manager_interface.hpp"

using orkhestrafs::dbmstodspi::QueryManagerInterface;

class MockQueryManager : public QueryManagerInterface {
 private:
  using ReuseLinks = std::map<std::string, std::map<int, MemoryReuseTargets>>;
  using MappedResultParameters =
      std::map<std::string, std::vector<StreamResultParameters>>;
  using ExecutionReadyNodes =
      std::pair<std::vector<AcceleratedQueryNode>, MappedResultParameters>;
  using ScheduledNodes =
      std::pair<ReuseLinks,
                std::queue<std::pair<ConfigurableModulesVector,
                                     std::vector<std::shared_ptr<QueryNode>>>>>;
  using MappedMemoryBlocks =
      std::unordered_map<std::string, MemoryBlockInterface*>;
  using MappedRecordSizes =
      std::map<std::string, std::vector<RecordSizeAndCount>>;
  using TableMap = std::map<std::string, TableMetadata>;
  using SchedulingNodeMap =
      std::unordered_map<std::string, SchedulingQueryNode>;
  using QueryNodeVector = std::vector<QueryNode*>;
  using Counter = std::unordered_map<std::string, int>;
  using Matrix = std::vector<std::vector<int>>;
  using HWLibrary = std::map<QueryOperationType, OperationPRModules>;

 public:
  MOCK_METHOD(std::vector<long>, GetData, (), (override));
  MOCK_METHOD(long, GetConfigTime, (), (override));
  MOCK_METHOD(void, MeasureBitstreamConfigurationSpeed,
              (const HWLibrary& hw_library,
               orkhestrafs::dbmstodspi::MemoryManagerInterface* memory_manager),
              (override));
  MOCK_METHOD(ReuseLinks, GetCurrentLinks,
              (std::queue<ReuseLinks> & all_reuse_links), (override));
  MOCK_METHOD(ExecutionReadyNodes, SetupAccelerationNodesForExecution,
              (DataManagerInterface * data_manager,
               MemoryManagerInterface* memory_manager,
               AcceleratorLibraryInterface* accelerator_library,
               const std::vector<QueryNode*>& current_query_nodes,
               const TableMap& current_tables_metadata,
               MappedMemoryBlocks& table_memory_blocks, Counter& table_counter),
              (override));
  MOCK_METHOD(void, LoadNextBitstreamIfNew,
              (MemoryManagerInterface * memory_manager,
               std::string bitstream_file_name, Config config),
              (override));
  /*MOCK_METHOD(bool, IsRunValid, (std::vector<AcceleratedQueryNode>
     current_run), (override));*/
  MOCK_METHOD(void, ExecuteAndProcessResults,
              (MemoryManagerInterface * memory_manager,
               FPGAManagerInterface* fpga_manager,
               const DataManagerInterface* data_manager,
               MappedMemoryBlocks& table_memory_blocks,
               const MappedResultParameters& result_parameters,
               const std::vector<AcceleratedQueryNode>& execution_query_nodes,
               TableMap& scheduling_table_data, Counter& table_counter,
               int timeout),
              (override));
  MOCK_METHOD(
      (std::queue<std::pair<std::vector<ScheduledModule>, QueryNodeVector>>),
      ScheduleNextSetOfNodes,
      (QueryNodeVector & query_nodes,
       const std::unordered_set<std::string>& first_node_names,
       const std::unordered_set<std::string>& starting_nodes,
       const SchedulingNodeMap& graph, TableMap& tables,
       AcceleratorLibraryInterface& drivers, const Config& config,
       NodeSchedulerInterface& node_scheduler,
       const std::vector<ScheduledModule>& current_configuration,
       std::unordered_set<std::string>& skipped_nodes, Counter& table_counter,
       const std::unordered_set<std::string>& blocked_nodes),
      (override));
  MOCK_METHOD(void, LoadInitialStaticBitstream,
              (MemoryManagerInterface * memory_manager, int clock_speed),
              (override));
  MOCK_METHOD(void, LoadEmptyRoutingPRRegion,
              (MemoryManagerInterface * memory_manager,
               AcceleratorLibraryInterface& driver_library),
              (override));
  MOCK_METHOD(long, LoadPRBitstreams,
              (MemoryManagerInterface * memory_manager,
               const std::vector<std::string>& bitstream_names,
               AcceleratorLibraryInterface& driver_library),
              (override));
  MOCK_METHOD((std::pair<std::vector<std::string>,
                         std::vector<std::pair<QueryOperationType, bool>>>),
              GetPRBitstreamsToLoadWithPassthroughModules,
              (std::vector<ScheduledModule> & current_config,
               const std::vector<ScheduledModule>& next_config,
               std::vector<std::string>& current_routing),
              (override));
  MOCK_METHOD((void), BenchmarkScheduling,
              (const std::unordered_set<std::string>& first_node_names,
               const std::unordered_set<std::string>& starting_nodes,
               std::unordered_set<std::string>& processed_nodes,
               const SchedulingNodeMap& graph, TableMap& tables,
               AcceleratorLibraryInterface& drivers, const Config& config,
               NodeSchedulerInterface& node_scheduler,
               std::vector<ScheduledModule>& current_configuration,
               const std::unordered_set<std::string>& blocked_nodes),
              (override));
  MOCK_METHOD((void), PrintBenchmarkStats, (), (override));
  MOCK_METHOD((int), GetRecordSizeFromParameters,
              (const DataManagerInterface* data_manager,
               const Matrix& node_parameters, int stream_index),
              (const override));
};