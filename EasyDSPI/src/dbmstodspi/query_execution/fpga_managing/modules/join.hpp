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
#include "join_interface.hpp"
#include "memory_manager_interface.hpp"

namespace easydspi::dbmstodspi {

/**
 * @brief Class to write in the join operation acceleration module settings.
 */
class Join : public AccelerationModule, public JoinInterface {
 private:
 public:
  ~Join() override = default;
  /**
   * @brief Constructor to set the memory manager instance to access memory
   * mapped registers.
   * @param memory_manager Memory manager instance to access memory mapping.
   * @param module_position Position of the module in the bitstream.
   */
  explicit Join(MemoryManagerInterface* memory_manager, int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * @brief Set stream parameters.
   * @param output_stream_chunk_count How many chunks of data are used for the
   * output data.
   * @param first_input_stream_id Input stream A.
   * @param second_input_stream_id Input stream B.
   * @param output_stream_id Output stream ID.
   */
  void DefineOutputStream(int output_stream_chunk_count,
                          int first_input_stream_id, int second_input_stream_id,
                          int output_stream_id) override;
  /**
   * @brief Set first input stream chunk count.
   * @param chunk_count How many chunks are used for each record.
   */
  void SetFirstInputStreamChunkCount(int chunk_count) override;
  /**
   * @brief Set first input stream chunk count.
   * @param chunk_count How many chunks are used for each record.
   */
  void SetSecondInputStreamChunkCount(int chunk_count) override;

  /**
   * @brief Configure the element selection after join.
   * @param output_chunk_id Which output chunk should the element be put to.
   * @param input_chunk_id Which input chunk should the element be taken from.
   * @param data_position Which position should be the element be taken from and
   * put to.
   * @param is_element_from_second_stream Is the next element going to be from
   * first or second stream.
   */
  void SelectOutputDataElement(int output_chunk_id, int input_chunk_id,
                               int data_position,
                               bool is_element_from_second_stream) override;

  /**
   * @brief Write into the module configuration to start prefetching data.
   */
  void StartPrefetchingData() override;

  /**
   * @brief Write into the reset register to reset the module.
   */
  void Reset() override;
};

}  // namespace easydspi::dbmstodspi