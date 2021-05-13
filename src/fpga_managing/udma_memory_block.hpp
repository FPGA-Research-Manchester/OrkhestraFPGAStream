#pragma once
#include <cstdint>

#include "memory_block_interface.hpp"
#include "udma.h"

namespace dbmstodspi::fpga_managing {

/**
 * @brief Memory mapped UDMA memory block for transferring data between the DDR
 * and the FPGA. Implements #MemoryBlockInterface
 */
class UDMAMemoryBlock : public MemoryBlockInterface {
 private:
  UdmaDevice* udma_device_{};

 public:
  ~UDMAMemoryBlock() override;
  explicit UDMAMemoryBlock(UdmaDevice* udma_device)
      : udma_device_(udma_device){};
  auto GetVirtualAddress() -> volatile uint32_t* override;
  auto GetPhysicalAddress() -> volatile uint32_t* override;
  auto GetSize() -> const uint32_t override;
};

}  // namespace dbmstodspi::fpga_managing
