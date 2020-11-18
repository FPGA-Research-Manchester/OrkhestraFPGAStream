#include "memory_manager.hpp"

#ifdef _FPGA_AVAILABLE
#include "udma_memory_block.hpp"
#else
#include "virtual_memory_block.hpp"
#endif

MemoryManager::~MemoryManager() = default;

MemoryManager::MemoryManager(const std::string& bitstream_name) { //NOLINT
#ifdef _FPGA_AVAILABLE
  acceleration_instance_ = pr_manager_.fpgaLoadStatic(bitstream_name);
  register_memory_block_ = acceleration_instance_.prmanager->accelRegs;
#else
  register_space_ = std::vector<uint32_t>((2 * 1024 * 1024), -1);
#endif
}

auto MemoryManager::AllocateMemoryBlock()
    -> std::unique_ptr<MemoryBlockInterface> {
#ifdef _FPGA_AVAILABLE
  return std::make_unique<UDMAMemoryBlock>(
      udma_repo_.device(memory_block_count_++));
#else
  return std::make_unique<VirtualMemoryBlock>();
#endif
}

auto MemoryManager::GetVirtualRegisterAddress(int offset)
    -> volatile uint32_t* {
#ifdef _FPGA_AVAILABLE
  return &(register_memory_block_[offset / 4]);
#else
  return &register_space_[offset];
#endif
}
