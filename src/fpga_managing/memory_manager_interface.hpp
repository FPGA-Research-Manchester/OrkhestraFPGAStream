#pragma once
#include <cstdint>
#include <memory>

#include "memory_block_interface.hpp"
class MemoryManagerInterface {
 public:
  virtual ~MemoryManagerInterface() = default;

  virtual auto AllocateMemoryBlock() -> std::unique_ptr<MemoryBlockInterface> = 0;
  virtual auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t* = 0;
};