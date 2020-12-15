#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <stack>

#include "memory_manager_interface.hpp"
#include "memory_block_interface.hpp"
#ifdef _FPGA_AVAILABLE
#include "cynq.h"
#include "udma.h"
#else
#include <vector>
#endif

class MemoryManager : public MemoryManagerInterface {
 private:
  std::stack<std::unique_ptr<MemoryBlockInterface>> available_memory_blocks_;
  int memory_block_count_ = -1;
  static const int kMaxPossibleAllocations = 8;
#ifdef _FPGA_AVAILABLE
  uint32_t* register_memory_block_;
  UdmaRepo udma_repo_;
  // Store to not delete the instances
  PRManager pr_manager_;
  StaticAccelInst acceleration_instance_;
#else
  std::vector<uint32_t> register_space_;
#endif
 public:
  ~MemoryManager() override;
  explicit MemoryManager(const std::string& bitstream_name,
                         int register_space_size);
  auto GetAvailableMemoryBlock() -> std::unique_ptr<MemoryBlockInterface> override;
  auto AllocateMemoryBlock() -> std::unique_ptr<MemoryBlockInterface> override;
  void FreeMemoryBlock(
      std::unique_ptr<MemoryBlockInterface> memory_block_pointer) override;
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t* override;
};
