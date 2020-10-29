#include "udma_memory_block.hpp"

UDMAMemoryBlock::~UDMAMemoryBlock() = default;

auto UDMAMemoryBlock::GetVirtualAddress() -> volatile uint32_t* {
  return reinterpret_cast<volatile uint32_t*>(udma_device_->map());
}

auto UDMAMemoryBlock::GetPhysicalAddress() -> volatile uint32_t* {
  return reinterpret_cast<volatile uint32_t*>(udma_device_->phys_addr);
}
