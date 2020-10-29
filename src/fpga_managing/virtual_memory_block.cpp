#include "virtual_memory_block.hpp"

VirtualMemoryBlock::~VirtualMemoryBlock() = default;

auto VirtualMemoryBlock::GetVirtualAddress() -> volatile uint32_t* {
  return &memory_area_[0];
}

auto VirtualMemoryBlock::GetPhysicalAddress() -> volatile uint32_t* {
  return &memory_area_[0];
}
