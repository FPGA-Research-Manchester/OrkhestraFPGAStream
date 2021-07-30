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
#include "multiplication_interface.hpp"

namespace dbmstodspi::fpga_managing::modules {

/**
 * @brief Class which implements low level memory writes to the multiplication
 * operation accelerator.
 *
 * This is for decimal(15,2) multiplication. Which means there is a loss of
 * accuracy since the results are floored not rounded.
 */
class Multiplication : public AccelerationModule,
                       public MultiplicationInterface {
 private:
 public:
  ~Multiplication() override = default;
  /**
   * @brief Constructor to set the memory manager instance to access memory
   * mapped registers.
   * @param memory_manager Memory manager instance to access memory mapping.
   * @param module_position Position of the module in the bitstream.
   */
  explicit Multiplication(MemoryManagerInterface* memory_manager,
                          int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * @brief Defube which streams need to have the multiplication operation.
   * @param active_streams Array of booleans noting active streams.
   */
  void DefineActiveStreams(std::bitset<16> active_streams) override;
  /**
   * @brief Define which positions in which chunk should have the multiplication
   * results.
   * @param chunk_id Which chunk is being configured.
   * @param active_positions Which positions should have the multiplication
   * result.
   */
  void ChooseMultiplicationResults(int chunk_id,
                                   std::bitset<8> active_positions) override;
};

}  // namespace dbmstodspi::fpga_managing::modules