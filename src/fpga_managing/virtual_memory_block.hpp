#pragma once
#include <cstdint>
#include <vector>

#include "memory_block_interface.hpp"
/**
 * Virtual memory block where there is no physical counterpart. Only for testing
 * and the data will be written to a virtual address. Implements
 * #MemoryBlockInterface.
 */
class VirtualMemoryBlock : public MemoryBlockInterface {
 private:
  std::vector<uint32_t> memory_area_;

 public:
  ~VirtualMemoryBlock() override;
  explicit VirtualMemoryBlock() : memory_area_(3 * 1024 * 1024){};
  auto GetVirtualAddress() -> volatile uint32_t* override;
  auto GetPhysicalAddress() -> volatile uint32_t* override;
  auto GetSize() -> const uint32_t override;
};
