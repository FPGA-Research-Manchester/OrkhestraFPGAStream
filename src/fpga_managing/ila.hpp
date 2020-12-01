#pragma once
#include <cstdint>

#include "ila_types.hpp"
#include "memory_manager_interface.hpp"
class ILA {
 public:
  explicit ILA(MemoryManagerInterface* memory_manager)
      : memory_manager_(memory_manager){};
  void startILAs();
  auto getValues(int clock_cycle, int location, ILADataTypes data_type)
      -> uint32_t;

 private:
  MemoryManagerInterface* memory_manager_;
  static auto calcAddress(int clock, int location, int offset) -> int;
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> uint32_t;
};