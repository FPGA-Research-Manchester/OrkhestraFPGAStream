#include "memory_block.hpp"

auto MemoryBlock::GetVirtualAddress() -> volatile uint32_t* {
  return reinterpret_cast<volatile uint32_t*>(udma_device_->map());
}

auto MemoryBlock::GetPhysicalAddress() -> volatile uint32_t* {
  return reinterpret_cast<volatile uint32_t*>(udma_device_->phys_addr);
}
