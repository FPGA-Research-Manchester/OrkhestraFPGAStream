#pragma once
#include "udma.h"
#include <cstdint>
class MemoryBlock
{
 private:
  UdmaDevice* udma_device_;
 public:
  explicit MemoryBlock(UdmaDevice* udma_device) : udma_device_(udma_device){};
  auto GetVirtualAddress() -> volatile uint32_t*;
  auto GetPhysicalAddress() -> volatile uint32_t*;
};
