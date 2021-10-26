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
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>;
  using MappedRecordSizes =
      std::map<std::string, std::vector<RecordSizeAndCount>>;

 public:
  MOCK_METHOD(
      ReuseLinks, GetCurrentLinks,
      (const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
       const ReuseLinks& all_reuse_links),
      (override));
  MOCK_METHOD(
      ExecutionReadyNodes, SetupAccelerationNodesForExecution,
      (DataManagerInterface * data_manager,
       MemoryManagerInterface* memory_manager,
       AcceleratorLibraryInterface* accelerator_library,
       MappedMemoryBlocks& input_memory_blocks,
       MappedMemoryBlocks& output_memory_blocks,
       MappedRecordSizes& input_stream_sizes,
       MappedRecordSizes& output_stream_sizes,
       const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes),
      (override));
  MOCK_METHOD(void, LoadNextBitstreamIfNew,
              (MemoryManagerInterface * memory_manager,
               std::string bitstream_file_name, Config config),
              (override));
  MOCK_METHOD(ScheduledNodes, ScheduleUnscheduledNodes,
              (std::vector<std::shared_ptr<QueryNode>> unscheduled_root_nodes,
               Config config, NodeSchedulerInterface& node_scheduler),
              (override));
  MOCK_METHOD(bool, IsRunValid, (std::vector<AcceleratedQueryNode> current_run),
              (override));
  MOCK_METHOD(void, ExecuteAndProcessResults,
              (FPGAManagerInterface * fpga_manager,
               const DataManagerInterface* data_manager,
               MappedMemoryBlocks& output_memory_blocks,
               MappedRecordSizes& output_stream_sizes,
               const MappedResultParameters& result_parameters,
               const std::vector<AcceleratedQueryNode>& execution_query_nodes),
              (override));
  MOCK_METHOD(void, FreeMemoryBlocks,
              (MemoryManagerInterface * memory_manager,
               MappedMemoryBlocks& input_memory_blocks,
               MappedMemoryBlocks& output_memory_blocks,
               MappedRecordSizes& input_stream_sizes,
               MappedRecordSizes& output_stream_sizes,
               const ReuseLinks& reuse_links,
               const std::vector<std::string>& scheduled_node_names),
              (override));
};