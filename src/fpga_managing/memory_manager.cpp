#include "memory_manager.hpp"
#include "cynq.h"

MemoryManager::MemoryManager(std::string bitstream_name) {
  PRManager prmanager;
  register_memory_block_ =
      prmanager.fpgaLoadStatic("DSPI_filtering").prmanager->accelRegs;
}

MemoryBlock MemoryManager::AllocateMemoryBlock() {
  return MemoryBlock(udma_repo_.device(memory_block_count_++));
}

volatile uint32_t* MemoryManager::GetVirtualRegisterAddress(int offset) {
  return &(register_memory_block_[offset / 4]);
}
