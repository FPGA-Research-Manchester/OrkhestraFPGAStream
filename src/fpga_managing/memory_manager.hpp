#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "memory_block_interface.hpp"
#ifdef _FPGA_AVAILABLE
#include "cynq.h"
#include "udma.h"
#else
#include <vector>
#endif

class MemoryManager {
 private:
#ifdef _FPGA_AVAILABLE
  int memory_block_count_ = 0;
  uint32_t* register_memory_block_;
  UdmaRepo udma_repo_;
  // Store to not delete the instances
  PRManager pr_manager_;
  StaticAccelInst acceleration_instance_;
#else
  std::vector<uint32_t> register_space_;
#endif
 public:
  explicit MemoryManager(std::string bitstream_name);
  auto AllocateMemoryBlock() -> std::unique_ptr<MemoryBlockInterface>;
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t*;
};
