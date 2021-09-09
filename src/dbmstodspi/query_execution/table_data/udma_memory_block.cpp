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

#include "udma_memory_block.hpp"

using orkhestrafs::dbmstodspi::UDMAMemoryBlock;

UDMAMemoryBlock::~UDMAMemoryBlock() = default;

// Needs to get rid of the volatile. Current stuff below doesn't make sense
auto UDMAMemoryBlock::GetVirtualAddress() -> volatile uint32_t* {
  intptr_t address = reinterpret_cast<intptr_t>(udma_device_->map());
  address += (-address) & 15;
  return reinterpret_cast<volatile uint32_t*>(address);
}

auto UDMAMemoryBlock::GetPhysicalAddress() -> volatile uint32_t* {
  intptr_t initial_address = reinterpret_cast<intptr_t>(udma_device_->map());
  intptr_t offset = (-initial_address) & 15;
  return reinterpret_cast<volatile uint32_t*>(udma_device_->phys_addr + offset);
}

auto UDMAMemoryBlock::GetSize() -> const uint32_t {
  intptr_t initial_address = reinterpret_cast<intptr_t>(udma_device_->map());
  intptr_t offset = (-initial_address) & 15;
  return udma_device_->size - offset;
}
