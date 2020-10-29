#include "memory_manager.hpp"

#include "udma_memory_block.hpp"

MemoryManager::MemoryManager(std::string bitstream_name) {
  acceleration_instance_ = pr_manager_.fpgaLoadStatic("DSPI_filtering");
  register_memory_block_ = acceleration_instance_.prmanager->accelRegs;
}

auto MemoryManager::AllocateMemoryBlock()
    -> std::unique_ptr<MemoryBlockInterface> {
  return std::make_unique<UDMAMemoryBlock>(
      udma_repo_.device(memory_block_count_++));
}

auto MemoryManager::GetVirtualRegisterAddress(int offset)
    -> volatile uint32_t* {
  return &(register_memory_block_[offset / 4]);
}
