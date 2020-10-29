#include "memory_manager.hpp"

MemoryManager::MemoryManager(std::string bitstream_name) {
  acceleration_instance_ = pr_manager_.fpgaLoadStatic("DSPI_filtering");
  register_memory_block_ = acceleration_instance_.prmanager->accelRegs;
}

MemoryBlock MemoryManager::AllocateMemoryBlock() {
  return MemoryBlock(udma_repo_.device(memory_block_count_++));
}

volatile uint32_t* MemoryManager::GetVirtualRegisterAddress(int offset) {
  return &(register_memory_block_[offset / 4]);
}
