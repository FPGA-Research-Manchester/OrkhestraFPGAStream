#include "udma_memory_block.hpp"

UDMAMemoryBlock::~UDMAMemoryBlock() = default;

// Needs to get rid of the volatile. Current stuff below doesn't make sense
auto UDMAMemoryBlock::GetVirtualAddress() -> volatile uint32_t* {
  intptr_t address = reinterpret_cast<intptr_t>(udma_device_->map());
  address += (-address) & 15;
  return reinterpret_cast<volatile uint32_t*>(address);
}

auto UDMAMemoryBlock::GetPhysicalAddress() -> volatile uint32_t* {
  intptr_t initial_address = reinterpret_cast<intptr_t>(udma_device_->map());
  intptr_t offset = (-initial_address) & 15;
  return reinterpret_cast<volatile uint32_t*>(udma_device_->phys_addr + offset);
}

auto UDMAMemoryBlock::GetSize() -> const uint32_t {
  intptr_t initial_address = reinterpret_cast<intptr_t>(udma_device_->map());
  intptr_t offset = (-initial_address) & 15;
  return udma_device_->size - offset;
}
