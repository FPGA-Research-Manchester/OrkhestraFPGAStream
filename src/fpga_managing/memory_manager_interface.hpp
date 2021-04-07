#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "memory_block_interface.hpp"
/**
 * @brief Interface class implemented in #MemoryManager
 */
class MemoryManagerInterface {
 public:
  virtual ~MemoryManagerInterface() = default;

  virtual void LoadBitstreamIfNew(const std::string& bitstream_name,
                                  int register_space_size) = 0;

  virtual auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t* = 0;

  virtual auto GetAvailableMemoryBlock()
      -> std::unique_ptr<MemoryBlockInterface> = 0;
  virtual void FreeMemoryBlock(
      std::unique_ptr<MemoryBlockInterface> memory_block_pointer) = 0;

 private:
  virtual auto AllocateMemoryBlock()
      -> std::unique_ptr<MemoryBlockInterface> = 0;
};