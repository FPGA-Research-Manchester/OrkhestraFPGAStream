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
#include "acceleration_module.hpp"
#include "memory_manager_interface.hpp"
#include "merge_sort_interface.hpp"

namespace dbmstodspi::fpga_managing::modules {

/**
 * @brief Class for writing the merge sort configuration data into the module
 * registers.
 */
class MergeSort : public AccelerationModule, public MergeSortInterface {
 public:
  ~MergeSort() override = default;
  /**
   * @brief Constructor to setup the merge sort memory access with the memory
   * manager instance.
   * @param memory_manager Memory manager instance to access memory mapped
   * registers.
   * @param module_position Position of the merge sorter in the bitstream
   */
  explicit MergeSort(MemoryManagerInterface* memory_manager,
                     int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * @brief Method to start prefetching data. Module instantiation data is
   * required to be passed.
   * @param base_channel_id The start channel for this module.
   * @param is_not_first_module Is this module the first module in the resource
   * elastic chain.
   */
  void StartPrefetchingData(int base_channel_id,
                            bool is_not_first_module) override;
  /**
   * @brief Set which stream is going to get sorted.
   * @param stream_id Stream to be sorted ID.
   * @param chunks_per_record How many chunks are used for each record.
   */
  void SetStreamParams(int stream_id, int chunks_per_record) override;
  /**
   * @brief Set how large is the buffer for this merge sorter.
   * @param record_count How many records could fit the buffer.
   */
  void SetBufferSize(int record_count) override;
  /**
   * @brief Set how many records should be fetched.
   * @param record_count How many records should be fetched.
   */
  void SetRecordCountPerFetch(int record_count) override;
  /**
   * @brief Set how many fetches should be made to fill the buffer.
   * @param fetch_count How many fetches should be made to fill the buffer.
   */
  void SetFetchCount(int fetch_count) override;
  /**
   * @brief Set what is the offset from trying to fill the buffer.
   * @param offset_record_count How much will still have to be fetched with the
   * last fetch.
   */
  void SetFetchOffset(int offset_record_count) override;
};

}  // namespace dbmstodspi::fpga_managing::modules