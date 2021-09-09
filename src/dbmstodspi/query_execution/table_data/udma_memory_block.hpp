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

#include "memory_block_interface.hpp"
#include "udma.h"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Memory mapped UDMA memory block for transferring data between the DDR
 * and the FPGA. Implements #MemoryBlockInterface
 */
class UDMAMemoryBlock : public MemoryBlockInterface {
 private:
  UdmaDevice* udma_device_{};

 public:
  ~UDMAMemoryBlock() override;
  explicit UDMAMemoryBlock(UdmaDevice* udma_device)
      : udma_device_(udma_device){};
  auto GetVirtualAddress() -> volatile uint32_t* override;
  auto GetPhysicalAddress() -> volatile uint32_t* override;
  auto GetSize() -> const uint32_t override;
};

}  // namespace dbmstodspi::fpga_managing
