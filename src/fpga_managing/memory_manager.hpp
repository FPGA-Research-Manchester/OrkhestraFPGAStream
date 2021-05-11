#pragma once
#include <cstdint>
#include <memory>
#include <stack>
#include <string>

#include "memory_block_interface.hpp"
#include "memory_manager_interface.hpp"
#ifdef _FPGA_AVAILABLE
#include "cynq.h"
#include "udma.h"
#else
#include <vector>
#endif

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Memory manager class to setup the FPGA memory accesses and to
 * facilitate the memory mapping.
 */
class MemoryManager : public MemoryManagerInterface {
 private:
  std::stack<std::unique_ptr<MemoryBlockInterface>> available_memory_blocks_;
  int memory_block_count_ = 0;
  // Set in
  // https://github.com/FPGA-Research-Manchester/fos/blob/fdac37e188e217293d296d9973c22500c8a4367c/udmalib/setupUdma.sh#L4
  static const int kMaxPossibleAllocations =
      6;  // The script is changed to allocate 6 X 280MB memory blocks.

  std::string loaded_bitstream_;
  int loaded_register_space_size_ = 0;
#ifdef _FPGA_AVAILABLE
  uint32_t* register_memory_block_;
  UdmaRepo udma_repo_;
  // Store to not delete the instances
  PRManager pr_manager_;
  StaticAccelInst acceleration_instance_;
#else
  std::vector<uint32_t> register_space_;
#endif
 public:
  ~MemoryManager() override;

  /**
   * @brief Load a new bitstream if it isn't loaded already.
   * @param bitstream_name Bitstream file path to be loaded.
   * @param register_space_size How much register memory space is required.
   */
  void LoadBitstreamIfNew(const std::string& bitstream_name,
                          int register_space_size) override;

  /**
   * @brief Get virtual register address given an offset from the base address.
   * @param offset Memory offset range from the base address.
   * @return Pointer to the required virtual memory.
   */
  auto GetVirtualRegisterAddress(int offset) -> volatile uint32_t* override;
  /**
   * @brief Get a pointer to a memory block which isn't used.
   * @return Pointer to a DDR memory block.
   */
  auto GetAvailableMemoryBlock()
      -> std::unique_ptr<MemoryBlockInterface> override;
  /**
   * @brief Mark a memory block as free.
   * @param memory_block_pointer Pointer to the unused DDR memory block.
   */
  void FreeMemoryBlock(
      std::unique_ptr<MemoryBlockInterface> memory_block_pointer) override;

 private:
  auto AllocateMemoryBlock() -> std::unique_ptr<MemoryBlockInterface> override;
  static void SetFPGAClockSpeed(int speed_value);
  static void SetFPGATo300MHz();
  static void SetFPGATo100MHz();
};

}  // namespace fpga_managing
}  // namespace dbmstodspi
