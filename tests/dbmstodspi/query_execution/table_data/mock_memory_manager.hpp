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

#include "gmock/gmock.h"
#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"

using orkhestrafs::core_interfaces::MemoryBlockInterface;
using orkhestrafs::dbmstodspi::MemoryManagerInterface;

class MockMemoryManager : public MemoryManagerInterface {
 public:
  void FreeMemoryBlock(
      std::unique_ptr<MemoryBlockInterface> memory_block_pointer) override {
    return MockFreeMemoryBlock(memory_block_pointer.get());
  }

  MOCK_METHOD(std::unique_ptr<MemoryBlockInterface>, GetAvailableMemoryBlock,
              (), (override));
  MOCK_METHOD(void, MockFreeMemoryBlock,
              (MemoryBlockInterface * memory_block_pointer), ());
  MOCK_METHOD(volatile uint32_t*, GetVirtualRegisterAddress, (int offset),
              (override));
  MOCK_METHOD(void, LoadBitstreamIfNew,
              (const std::string& bitstream_name,
               const int register_space_size),
              (override));
  MOCK_METHOD(void, LoadStatic, (), (override));
  MOCK_METHOD(void, LoadPartialBitstream,
              (const std::string& bitstream_name, DMAInterface& dma_engine),
              (override));

 private:
  MOCK_METHOD(std::unique_ptr<MemoryBlockInterface>, AllocateMemoryBlock, (),
              (override));
};