#pragma once
#include <cstdint>
#include <string>

#include "memory_block.hpp"
#include "udma.h"
class MemoryManager {
 private:
  int memory_block_count_ = 0;
  UdmaRepo udma_repo_;
  uint32_t* register_memory_block_;

 public:
  explicit MemoryManager(std::string bitstream_name);
  auto AllocateMemoryBlock() -> MemoryBlock;
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t*;
};
