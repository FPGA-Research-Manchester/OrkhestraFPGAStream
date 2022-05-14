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
#include <memory>
#include <string>
#include <vector>

#include "dma_interface.hpp"
#include "memory_block_interface.hpp"

using orkhestrafs::core_interfaces::MemoryBlockInterface;
using orkhestrafs::dbmstodspi::DMAInterface;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Interface class implemented in #MemoryManager
 */
class MemoryManagerInterface {
 public:
  virtual ~MemoryManagerInterface() = default;

  virtual void LoadBitstreamIfNew(const std::string& bitstream_name,
                                  int register_space_size) = 0;

  virtual auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t* = 0;

  virtual auto GetAvailableMemoryBlock()
      -> MemoryBlockInterface* = 0;
  virtual void FreeMemoryBlock(
      MemoryBlockInterface* memory_block_pointer) = 0;
  virtual void LoadStatic() = 0;
  virtual void LoadPartialBitstream(
      const std::vector<std::string>& bitstream_name,
      DMAInterface& dma_engine) = 0;

 private:
  virtual auto AllocateMemoryBlock()
      -> MemoryBlockInterface* = 0;
};

}  // namespace orkhestrafs::dbmstodspi