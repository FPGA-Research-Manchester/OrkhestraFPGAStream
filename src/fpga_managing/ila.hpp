#pragma once
#include <cstdint>

#include "ila_types.hpp"
#include "memory_manager_interface.hpp"
/**
 * @brief Debug class to access ILA values if is in the used bitstream.
 */
class ILA {
 public:
  /**
   * @brief Constructor to setup memory mapped registers.
   * @param memory_manager Instance with which it is possible to access memory
   * mapped registers.
   */
  explicit ILA(MemoryManagerInterface* memory_manager)
      : memory_manager_(memory_manager){};
  /**
   * @brief Start ILA data collection.
   */
  void StartILAs();
  /**
   * @brief Start ILA data collection at the AXI ports.
   */
  void StartAxiILA();
  /**
   * @brief Get data ILA has been collecting.
   * @param clock_cycle Which clock cycle data should be read.
   * @param location What ILA location should be read.
   * @param data_type Which data should be read.
   * @return The data ILA has been collecting.
   */
  auto GetValues(int clock_cycle, int location, ILADataTypes data_type)
      -> uint32_t;
  /**
   * @brief Print all data ILA has been collecting.
   * @param ila_id ID of the ILA
   * @param max_clock How many clock cycles of data there is.
   */
  void PrintILAData(int ila_id, int max_clock);
  /**
   * @brief Print all ILA data at the AXI ports.
   * @param max_clock How many clock cycles of data there is.
   */
  void PrintAxiILAData(int max_clock);
  /**
   * @brief Print all ILA data at the DMA location.
   * @param max_clock How many clock cycles of data there is.
   */
  void PrintDMAILAData(int max_clock);

 private:
  MemoryManagerInterface* memory_manager_;
  static auto CalcAddress(int clock, int ila_id, int offset) -> int;
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> uint32_t;
};