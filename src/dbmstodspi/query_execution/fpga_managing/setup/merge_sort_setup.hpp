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
#include "acceleration_module_setup_interface.hpp"
#include "blocking_sort_module_setup.hpp"
#include "first_module_setup.hpp"
#include "merge_sort_interface.hpp"
#include "sorting_module_setup.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to calculate the configuration data and setup the merge sorting
 * acceleration.
 */
class MergeSortSetup : public virtual AccelerationModuleSetupInterface,
                       public FirstModuleSetup,
                       public SortingModuleSetup,
                       public BlockingSortModuleSetup {
 public:
  void SetupModule(AccelerationModule& acceleration_module,
                   const AcceleratedQueryNode& module_parameters) override;
  auto CreateModule(MemoryManagerInterface* memory_manager, int module_position)
      -> std::unique_ptr<AccelerationModule> override;
  auto IsMultiChannelStream(bool is_input_stream, int stream_index)
      -> bool override;
  auto GetMultiChannelParams(bool is_input, int stream_index,
                             std::vector<std::vector<int>> operation_parameters)
      -> std::pair<int, std::vector<int>> override;
  auto IsDataSensitive() -> bool override;
  auto GetMinSortingRequirementsForTable(const TableMetadata& table_data)
      -> std::vector<int> override;
  /*auto IsSortingInputTable() -> bool override;*/
  auto SetMissingFunctionalCapacity(const std::vector<int>& bitstream_capacity,
                                    std::vector<int>& missing_capacity,
                                    const std::vector<int>& node_capacity,
                                    bool is_composed) -> bool override;
  /**
   * @brief Calculate the correct configuration data and write the setup data
   * into the memory mapped registers.
   * @param merge_sort_module Merge sort module instance to access the memory
   * mapped registers.
   * @param stream_id ID of the stream to be sorted.
   * @param record_size How many integers worth of data there is in each record.
   * @param base_channel_id The channel ID where this module starts sorting.
   * @param is_first Is this the first module in the resource elastic chain.
   */
  static void SetupMergeSortModule(MergeSortInterface& merge_sort_module,
                                   int stream_id, int record_size,
                                   int base_channel_id, bool is_first,
                                   int module_size, int dma_record_size);
  /**
   * @brief Calculate how many records are fetched each time.
   * @param sort_buffer_size Buffer size of the merge sort module.
   * @param record_size How many integers worth of data there is in a record.
   * @return How many records are fetched.
   */
  static auto CalculateRecordCountPerFetch(int sort_buffer_size,
                                           int module_record_size,
                                           int dma_record_size) -> int;
  /**
   * @brief Calculate how big the buffer size has to be.
   * @param buffer_space How much space there is available in integers.
   * @param channel_count How many channels are used.
   * @param chunks_per_record How many chunks are used for each record.
   * @return How many records can fit into the buffer.
   */
  static auto CalculateSortBufferSize(int buffer_space, int channel_count,
                                      int chunks_per_record) -> int;

 private:
  static auto PotentialRecordCountIsValid(int potential_record_count,
                                          int record_size) -> bool;
  static void SetupPassthroughMergeSort(MergeSortInterface& merge_sort_module);
};

}  // namespace orkhestrafs::dbmstodspi