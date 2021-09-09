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
#include <vector>

#include "memory_block_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Virtual memory block where there is no physical counterpart.
 *
 * Only for testing and the data will be written to a virtual address.
 * Implements #MemoryBlockInterface.
 */
class VirtualMemoryBlock : public MemoryBlockInterface {
 private:
  std::vector<uint32_t> memory_area_;

 public:
  ~VirtualMemoryBlock() override;
  explicit VirtualMemoryBlock() : memory_area_(3 * 1024 * 1024){};
  auto GetVirtualAddress() -> volatile uint32_t* override;
  auto GetPhysicalAddress() -> volatile uint32_t* override;
  auto GetSize() -> const uint32_t override;
};

}  // namespace orkhestrafs::dbmstodspi