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
#include "accelerator_library_interface.hpp"

using orkhestrafs::core_interfaces::Config;
using orkhestrafs::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using orkhestrafs::core_interfaces::query_scheduling_data::MemoryReuseTargets;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;
using orkhestrafs::core_interfaces::query_scheduling_data::RecordSizeAndCount;
using orkhestrafs::core_interfaces::query_scheduling_data::
    StreamResultParameters;
using orkhestrafs::dbmstodspi::AcceleratedQueryNode;
using orkhestrafs::dbmstodspi::FPGAManagerInterface;
using orkhestrafs::dbmstodspi::MemoryBlockInterface;
using orkhestrafs::dbmstodspi::AcceleratorLibraryInterface;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface to describe a class managing the setup and execution of a
 * query
 */
class QueryManagerInterface {
 public:
  virtual ~QueryManagerInterface() = default;
  /**
   * @brief Get memory dependencies in the current run to save memory block
   * pointers for next runs.
   * @param current_query_nodes Current nodes to be executed.
   * @param all_reuse_links All links for currently scheduled nodes.
   * @return Map of current memory reuse targets and for which stream they
   * should be saved for.
   */
  virtual auto GetCurrentLinks(
      const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          all_reuse_links)
      -> std::map<std::string, std::map<int, MemoryReuseTargets>> = 0;
  /**
   * @brief Method to create nodes for execution from scheduled nodes.
   * @param data_manager Manager to handle table data and different data types.
   * @param memory_manager Manager to handle memory blocks.
   * @param accelerator_library Library of accelerators and their drivers.
   * @param input_memory_blocks Memory for input streams.
   * @param output_memory_blocks Memory for output streams.
   * @param input_stream_sizes Map for input stream size parameters.
   * @param output_stream_sizes Map for output stream size parameters.
   * @param current_query_nodes Currently scheduled nodes.
   * @return Nodes ready for execution with saved parameters for reading out
   * results.
   */
  virtual auto SetupAccelerationNodesForExecution(
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
          std::map<std::string, std::vector<StreamResultParameters>>> = 0;
  /**
   * @brief Load a bitstream
   * @param memory_manager Memory manager for accessing FOS and loading the
   * bitstream.
   * @param bitstream_file_name Bitstream to load
   * @param config Configuration for how much addressable memory is usable.
   */
  virtual void LoadNextBitstreamIfNew(MemoryManagerInterface* memory_manager,
                                      std::string bitstream_file_name,
                                      Config config) = 0;
  /**
   * @brief Schedule operations to different runs.
   * @param unscheduled_root_nodes Nodes with no dependencies.
   * @param config Configuration for different operators.
   * @return How memory pointers could be reused between differnt runs and a
   * queue of runs.
   */
  virtual auto ScheduleUnscheduledNodes(
      std::vector<std::shared_ptr<QueryNode>> unscheduled_root_nodes,
      Config config)
      -> std::pair<
          std::map<std::string, std::map<int, MemoryReuseTargets>>,
          std::queue<std::pair<ConfigurableModulesVector,
                               std::vector<std::shared_ptr<QueryNode>>>>> = 0;
  /**
   * @brief Check if the run is correctly scheduled.
   * @param current_run Nodes ready for execution.
   * @return Boolean flag noting if the run is valid.
   */
  virtual auto IsRunValid(std::vector<AcceleratedQueryNode> current_run)
      -> bool = 0;
  /**
   * @brief Execute given nodes.
   * @param fpga_manager Manager to setup and operate modules.
   * @param data_manager Manager to handle table data for result reading.
   * @param output_memory_blocks Memory for output streams.
   * @param output_stream_sizes Map for output stream size parameters.
   * @param result_parameters Saved parameters for result reading.
   * @param execution_query_nodes Nodes to execute
   */
  virtual void ExecuteAndProcessResults(
      FPGAManagerInterface* fpga_manager,
      const DataManagerInterface* data_manager,
      std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<AcceleratedQueryNode>& execution_query_nodes) = 0;

  /**
   * @brief Method to move reusable output memory blocks to input maps. And the
   * rest of the memory blocks get freed.
   * @param memory_manager Manager to handle memory blocks and reading and
   * writing to memory mapped addresses.
   * @param input_memory_blocks Memory for input streams.
   * @param output_memory_blocks Memory for output streams.
   * @param input_stream_sizes Map for input stream size parameters.
   * @param output_stream_sizes Map for output stream size parameters.
   * @param reuse_links Map for reusing memory blocks for next runs.
   * @param scheduled_node_names Names of the currently executed nodes.
   */
  virtual void FreeMemoryBlocks(
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
      const std::vector<std::string>& scheduled_node_names) = 0;
};

}  // namespace orkhestrafs::dbmstodspi