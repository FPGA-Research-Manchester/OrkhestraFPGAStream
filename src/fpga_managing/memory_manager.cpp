#include "memory_manager.hpp"


MemoryManager::MemoryManager(std::string bitstream_name) {
  PRManager prmanager;
  acceleration_instance_ = prmanager.fpgaLoadStatic("DSPI_filtering");
}

MemoryBlock MemoryManager::AllocateMemoryBlock() {
  return MemoryBlock(udma_repo_.device(memory_block_count_++));
}

volatile uint32_t* MemoryManager::GetVirtualRegisterAddress(int offset) {
  return &(acceleration_instance_.prmanager->accelRegs[offset / 4]);
}
