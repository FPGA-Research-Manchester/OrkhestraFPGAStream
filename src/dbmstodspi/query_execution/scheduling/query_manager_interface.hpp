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
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "accelerated_query_node.hpp"
#include "accelerator_library_interface.hpp"
#include "config.hpp"
#include "data_manager_interface.hpp"
#include "fpga_manager_interface.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
#include "node_scheduler_interface.hpp"
#include "query_scheduling_data.hpp"
#include "scheduled_module.hpp"

using orkhestrafs::core_interfaces::Config;
using orkhestrafs::core_interfaces::MemoryBlockInterface;
using orkhestrafs::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using orkhestrafs::core_interfaces::query_scheduling_data::MemoryReuseTargets;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;
using orkhestrafs::core_interfaces::query_scheduling_data::RecordSizeAndCount;
using orkhestrafs::core_interfaces::query_scheduling_data::
    StreamResultParameters;
using orkhestrafs::dbmstodspi::AcceleratedQueryNode;
using orkhestrafs::dbmstodspi::AcceleratorLibraryInterface;
using orkhestrafs::dbmstodspi::FPGAManagerInterface;
using orkhestrafs::dbmstodspi::NodeSchedulerInterface;
using orkhestrafs::dbmstodspi::ScheduledModule;

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
   * @param all_reuse_links All links for currently scheduled nodes.
   * @return Map of current memory reuse targets and for which stream they
   * should be saved for.
   */
  virtual auto GetCurrentLinks(
      std::queue<std::map<std::string, std::map<int, MemoryReuseTargets>>>&
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
      const std::vector<QueryNode*>& current_query_nodes, const std::map<std::string, TableMetadata>& current_tables_metadata,
      std::map<std::string, MemoryBlockInterface>& table_memory_blocks)
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
   * @brief Check if the run is correctly scheduled.
   * @param current_run Nodes ready for execution.
   * @return Boolean flag noting if the run is valid.
   */
  // virtual auto IsRunValid(std::vector<AcceleratedQueryNode> current_run)
  //    -> bool = 0;
  /**
   * @brief Execute given nodes.
   * @param fpga_manager Manager to setup and operate modules.
   * @param data_manager Manager to handle table data for result reading.
   * @param output_memory_blocks Memory for output streams.
   * @param output_stream_sizes Map for output stream size parameters.
   * @param result_parameters Saved parameters for result reading.
   * @param execution_query_nodes Nodes to execute
   * @param scheduling_table_data Table sizes data for scheduling.
   * @param reuse_links To find next nodes.
   * @param scheduling_graph To update next nodes.
   */
  virtual void ExecuteAndProcessResults(
      FPGAManagerInterface* fpga_manager,
      const DataManagerInterface* data_manager,
      std::map<std::string, std::vector<MemoryBlockInterface*>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<AcceleratedQueryNode>& execution_query_nodes,
      std::map<std::string, TableMetadata>& scheduling_table_data,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          reuse_links,
      std::unordered_map<std::string, SchedulingQueryNode>&
          scheduling_graph) = 0;

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
      std::map<std::string, std::vector<MemoryBlockInterface*>>&
          input_memory_blocks,
      std::map<std::string, std::vector<MemoryBlockInterface*>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      const std::map<std::string, std::map<int, MemoryReuseTargets>>&
          reuse_links,
      const std::vector<std::string>& scheduled_node_names) = 0;

  /**
   * Method to schedule next set of nodes based on PR graph nodes.
   * @param query_nodes Root query nodes
   * @param first_node_names Node names which are constrained to first pos
   * @param starting_nodes Available query node names
   * @param processed_nodes Processed node names
   * @param graph Graph of all node names
   * @param tables Map of all tables
   * @param drivers Driver library
   * @param config Config values
   * @param node_scheduler The scheduler object
   * @param all_reuse_links All links between runs.
   * @param current_configuration Currently configured bitstreams
   * @return Queue of sets of runs.
   */
  virtual auto ScheduleNextSetOfNodes(
      std::vector<QueryNode*>& query_nodes,
      const std::unordered_set<std::string>& first_node_names,
      const std::unordered_set<std::string>& starting_nodes,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::map<std::string, TableMetadata>& tables,
      AcceleratorLibraryInterface& drivers, const Config& config,
      NodeSchedulerInterface& node_scheduler,
      const std::vector<ScheduledModule>& current_configuration,
      std::unordered_set<std::string>& skipped_nodes)
      -> std::queue<std::pair<std::vector<ScheduledModule>,
                              std::vector<QueryNode*>>> = 0;

  virtual void BenchmarkScheduling(
      const std::unordered_set<std::string>& first_node_names,
      const std::unordered_set<std::string>& starting_nodes,
      std::unordered_set<std::string>& processed_nodes,
      const std::unordered_map<std::string, SchedulingQueryNode>& graph,
      std::map<std::string, TableMetadata>& tables,
      AcceleratorLibraryInterface& drivers, const Config& config,
      NodeSchedulerInterface& node_scheduler,
      std::vector<ScheduledModule>& current_configuration) = 0;

  /**
   * @brief Load the initial static system
   * @param memory_manager Memory manager for accessing the FPGA and loading the
   * bitstream.
   */
  virtual void LoadInitialStaticBitstream(
      MemoryManagerInterface* memory_manager) = 0;

  /**
   * @brief Load the empty PR region routing wires
   * @param memory_manager Memory manager for accessing the FPGA and loading the
   * bitstream.
   * @param driver_library To get the DMA module to decouple from the PR region.
   */
  virtual void LoadEmptyRoutingPRRegion(
      MemoryManagerInterface* memory_manager,
      AcceleratorLibraryInterface& driver_library) = 0;

  /**
   * @brief Load PR bitstreams
   * @param memory_manager Memory manager for accessing the FPGA and loading the
   * bitstream.
   * @param bitstream_names Bitstreams to load
   * @param driver_library To get the DMA module to decouple from the PR region.
   */
  virtual void LoadPRBitstreams(
      MemoryManagerInterface* memory_manager,
      const std::vector<std::string>& bitstream_names,
      AcceleratorLibraryInterface& driver_library) = 0;

  /**
   * @brief Find out which bitstreams have to be loaded next.
   * @param current_config Currently loaded modules.
   * @param next_config Desired modules.
   * @param column_count how many columns there are in the PR region.
   * @return Which bitstreams are required and which modules can be skipped.
   */
  virtual auto GetPRBitstreamsToLoadWithPassthroughModules(
      std::vector<ScheduledModule>& current_config,
      const std::vector<ScheduledModule>& next_config, int column_count)
      -> std::pair<std::vector<std::string>,
                   std::vector<std::pair<QueryOperationType, bool>>> = 0;

  virtual void PrintBenchmarkStats() = 0;
  virtual auto GetRecordSizeFromParameters(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& node_parameters,
      int stream_index) const
      ->int ;
};

}  // namespace orkhestrafs::dbmstodspi