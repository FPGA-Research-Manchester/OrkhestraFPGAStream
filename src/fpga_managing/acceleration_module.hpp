#pragma once
#include <cstdint>

#include "memory_manager_interface.hpp"

/**
 * Base abstract class which all acceleration modules have to extend to be able
 * to access memory mapped registers
 */
class AccelerationModule {
 private:
  /// Memory manager instance to be able to access the memory mapped register
  /// space.
  MemoryManagerInterface* memory_manager_;
  /// Location of the module on the FPGA.
  const int module_position_;

  /**
   * Calculate where the virtual memory address is memory mapped to. The result
   *  depends on the module position.
   * @param module_internal_address Internal virtual address which is desired to
   *  be accessed.
   * @return Global memory mapped address which can be read or written to.
   */
  auto CalculateMemoryMappedAddress(int module_internal_address)
      -> volatile uint32_t*;

 protected:
  /**
   * Write data to a module configuration register.
   * @param module_internal_address Internal address of the register.
   * @param write_data Data to be written to the register.
   */
  void WriteToModule(int module_internal_address, uint32_t write_data);
  /**
   * Read data from a module configuration register.
   * @param module_internal_address Internal address of the register
   * @return Data read from the register.
   */
  auto ReadFromModule(int module_internal_address) -> volatile uint32_t;
  /**
   * Constructor to pass the memory manager instance and the module position
   *  information.
   * @param memory_manager Memory manager instance to access memory mapped
   *  registers.
   * @param module_position Integer showing the position of the module on the
   *  FPGA.
   */
  AccelerationModule(MemoryManagerInterface* memory_manager,
                     int module_position)
      : memory_manager_(memory_manager), module_position_(module_position){};

 public:
  virtual ~AccelerationModule() = 0;
};
