#pragma once
#include "memory_block_interface.hpp"
#include "udma.h"
#include <cstdint>
class UDMAMemoryBlock : public MemoryBlockInterface {
 private:
  UdmaDevice* udma_device_{};

 public:
  ~UDMAMemoryBlock() override;
  explicit UDMAMemoryBlock(UdmaDevice* udma_device) : udma_device_(udma_device){};
  auto GetVirtualAddress() -> volatile uint32_t* override;
  auto GetPhysicalAddress() -> volatile uint32_t* override;
  auto GetSize() -> const uint32_t override;
};
