#pragma once
#include <cstdint>

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Interface class for accessing DDR memory where the table data is
 * stored.
 */
class MemoryBlockInterface {
 public:
  virtual ~MemoryBlockInterface() = default;

  /**
   * @brief Get the virtual address for modifying the data in the memory.
   * @return Pointer to the start of the virtual address block.
   */
  virtual auto GetVirtualAddress() -> volatile uint32_t* = 0;
  /**
   * @brief Get the physical address to configure the hardware with.
   * @return Pointer to the start of the physical address block.
   */
  virtual auto GetPhysicalAddress() -> volatile uint32_t* = 0;
  /**
   * @brief Get the size of the allocated memory block.
   * @return How much data can be written into this memory block in integers.
   */
  virtual auto GetSize() -> const uint32_t = 0;
};

}  // namespace fpga_managing
}  // namespace dbmstodspi