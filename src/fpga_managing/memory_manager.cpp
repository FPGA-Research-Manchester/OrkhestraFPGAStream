#include "memory_manager.hpp"

#include <stdexcept>

#ifdef _FPGA_AVAILABLE
#include "udma_memory_block.hpp"
#else
#include "virtual_memory_block.hpp"
#endif

MemoryManager::~MemoryManager() = default;

void MemoryManager::LoadBitstreamIfNew(const std::string& bitstream_name,
                                       const int register_space_size) {
  if (bitstream_name != loaded_bitstream_ &&
      register_space_size != loaded_register_space_size_) {
#ifdef _FPGA_AVAILABLE
    acceleration_instance_ =
        pr_manager_.fpgaLoadStatic(bitstream_name, register_space_size);
    register_memory_block_ = acceleration_instance_.prmanager->accelRegs;
#else
    register_space_ = std::vector<uint32_t>(register_space_size, -1);
#endif
    loaded_register_space_size_ = register_space_size;
    loaded_bitstream_ = bitstream_name;
  }
}

auto MemoryManager::GetAvailableMemoryBlock()
    -> std::unique_ptr<MemoryBlockInterface> {
  if (available_memory_blocks_.empty()) {
    return AllocateMemoryBlock();
  }
  std::unique_ptr<MemoryBlockInterface> available_memory_block =
      std::move(available_memory_blocks_.top());
  available_memory_blocks_.pop();
  return available_memory_block;
}

auto MemoryManager::GetVirtualRegisterAddress(int offset)
    -> volatile uint32_t* {
  if (!loaded_register_space_size_) {
    throw std::runtime_error("No bitstream loaded!");
  }
#ifdef _FPGA_AVAILABLE
  return &(register_memory_block_[offset / 4]);
#else
  return &(register_space_[offset]);
#endif
}

auto MemoryManager::AllocateMemoryBlock()
    -> std::unique_ptr<MemoryBlockInterface> {
  if (memory_block_count_++ >= kMaxPossibleAllocations) {
    throw std::runtime_error("Can't allocate any more memory!");
  }
#ifdef _FPGA_AVAILABLE
  return std::make_unique<UDMAMemoryBlock>(
      udma_repo_.device(memory_block_count_-1));
#else
  return std::make_unique<VirtualMemoryBlock>();
#endif
}

void MemoryManager::FreeMemoryBlock(
    std::unique_ptr<MemoryBlockInterface> memory_block_pointer) {
  available_memory_blocks_.push(std::move(memory_block_pointer));
}