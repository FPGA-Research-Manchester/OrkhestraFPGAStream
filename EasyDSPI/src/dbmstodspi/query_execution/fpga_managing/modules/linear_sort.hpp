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
#include "linear_sort_interface.hpp"
#include "memory_manager_interface.hpp"

namespace easydspi::dbmstodspi {

/**
 * @brief Class to implement the linear sorting operation acceleration.
 */
class LinearSort : public AccelerationModule, public LinearSortInterface {
 private:
 public:
  ~LinearSort() override = default;
  /**
   * @brief Constructor to setup memory mapper access to configuration
   * registers.
   * @param memory_manager Memory manager instance to access memory mapped
   * registers.
   * @param module_position Position of this module in the bitstream.
   */
  explicit LinearSort(MemoryManagerInterface* memory_manager,
                      int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * @brief Set stream parameters such that the module knows which stream ID to
   * look for.
   * @param stream_id Stream to be sorted.
   * @param chunks_per_record How many chunks are used for each record in the
   *  stream.
   */
  void SetStreamParams(int stream_id, int chunks_per_record) override;

  /**
   * @brief Write to the module to start prefetching data.
   */
  void StartPrefetchingData() override;
};

}  // namespace easydspi::dbmstodspi