#pragma once
#include <string>
#include "memory_block.hpp"
#include <cstdint>
#include "cynq.h"
#include "udma.h"
class MemoryManager
{
 private:
  int memory_block_count_ = 0;
  UdmaRepo udma_repo_;
  StaticAccelInst acceleration_instance_;
 public:
  explicit MemoryManager(std::string bitstream_name);
  auto AllocateMemoryBlock() -> MemoryBlock;
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t*;
};
