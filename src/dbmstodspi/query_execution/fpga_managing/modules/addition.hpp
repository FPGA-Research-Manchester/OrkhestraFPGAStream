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
#include "addition_interface.hpp"
#include "memory_manager_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class which implements low level memory writes to the addition and
 * subtraction operator accelerator.
 */
class Addition : public AccelerationModule, public AdditionInterface {
 private:
 public:
  ~Addition() override = default;
  /**
   * @brief Constructor to set the memory manager instance to access memory
   * mapped registers.
   * @param memory_manager Memory manager instance to access memory mapping.
   * @param module_position Position of the module in the bitstream.
   */
  explicit Addition(MemoryManagerInterface* memory_manager, int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * @brief Define the input stream and which chunk from the input stream should
   * get modified.
   * @param stream_id Stream to be operated on.
   * @param chunk_id Chunk to be operated on.
   */
  void DefineInput(int stream_id, int chunk_id) override;
  /**
   * @brief Set which sign the input should have.
   * @param is_value_negative Array of booleans noting negative input values.
   */
  void SetInputSigns(std::bitset<8> is_value_negative) override;
  /**
   * @brief Set the literal constant values for addition.
   * @param literal_values Array of 64-bit values.
   */
  void SetLiteralValues(
      std::array<std::pair<uint32_t, uint32_t>, 8> literal_values) override;
};

}  // namespace orkhestrafs::dbmstodspi