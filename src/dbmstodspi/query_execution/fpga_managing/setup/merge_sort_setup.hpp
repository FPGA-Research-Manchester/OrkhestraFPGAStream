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
#include "merge_sort_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to calculate the configuration data and setup the merge sorting
 * acceleration.
 */
class MergeSortSetup {
 public:
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
                                   int base_channel_id, bool is_first);
  /**
   * @brief Calculate how many records are fetched each time.
   * @param sort_buffer_size Buffer size of the merge sort module.
   * @param record_size How many integers worth of data there is in a record.
   * @return How many records are fetched.
   */
  static auto CalculateRecordCountPerFetch(int sort_buffer_size,
                                           int record_size) -> int;
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
};

}  // namespace orkhestrafs::dbmstodspi