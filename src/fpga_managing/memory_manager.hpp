#pragma once
#include <cstdint>
#include <string>

#include "cynq.h"
#include "memory_block.hpp"
#include "udma.h"
class MemoryManager {
 private:
  int memory_block_count_ = 0;
  UdmaRepo udma_repo_;
  uint32_t* register_memory_block_;
  // Store to not delete the instances
  PRManager pr_manager_;
  StaticAccelInst acceleration_instance_;

 public:
  explicit MemoryManager(std::string bitstream_name);
  auto AllocateMemoryBlock() -> MemoryBlock;
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t*;
};
