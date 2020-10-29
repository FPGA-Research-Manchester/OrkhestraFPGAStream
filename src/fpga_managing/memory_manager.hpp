#pragma once
#include <cstdint>
#include <string>
#include <memory>

#include "memory_block_interface.hpp"
#include "cynq.h"
#include "udma.h"
class MemoryManager {
 private:
  int memory_block_count_ = 0;
  uint32_t* register_memory_block_;

  UdmaRepo udma_repo_;
  // Store to not delete the instances
  PRManager pr_manager_;
  StaticAccelInst acceleration_instance_;

 public:
  explicit MemoryManager(std::string bitstream_name);
  auto AllocateMemoryBlock() -> std::unique_ptr<MemoryBlockInterface>;
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t*;
};
