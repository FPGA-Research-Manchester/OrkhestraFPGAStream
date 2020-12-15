#pragma once
#include <cstdint>
#include <memory>

#include "gmock/gmock.h"
#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
class MockMemoryManager : public MemoryManagerInterface {
 public:
  MOCK_METHOD(std::unique_ptr<MemoryBlockInterface>, GetAvailableMemoryBlock, (),
              (override));
  MOCK_METHOD(std::unique_ptr<MemoryBlockInterface>, AllocateMemoryBlock,
              (), (override));
  MOCK_METHOD(void, FreeMemoryBlock,
              (std::unique_ptr<MemoryBlockInterface> memory_block_pointer),
              (override));
  MOCK_METHOD(volatile uint32_t*, GetVirtualRegisterAddress, (int offset),
              (override));
};