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
#include <cstdint>

namespace orkhestrafs::core_interfaces {

/**
 * @brief Interface class for accessing DDR memory where the table data is
 * stored.
 */
class MemoryBlockInterface {
 public:
  virtual ~MemoryBlockInterface() = default;

  /**
   * @brief Get the virtual address for modifying the data in the memory.
   * @return Pointer to the start of the virtual address block.
   */
  virtual auto GetVirtualAddress() -> volatile uint32_t* = 0;
  /**
   * @brief Get the physical address to configure the hardware with.
   * @return Pointer to the start of the physical address block.
   */
  virtual auto GetPhysicalAddress() -> volatile uint32_t* = 0;
  /**
   * @brief Get the size of the allocated memory block.
   * @return How much data can be written into this memory block in integers.
   */
  virtual auto GetSize() -> const uint32_t = 0;
};

}  // namespace orkhestrafs::dbmstodspi