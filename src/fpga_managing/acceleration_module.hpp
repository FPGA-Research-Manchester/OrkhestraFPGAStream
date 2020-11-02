#pragma once
#include <cstdint>

#include "memory_manager_interface.hpp"

class AccelerationModule {
 private:
  MemoryManagerInterface* memory_manager_;
  const int module_position_;

  auto CalculateMemoryMappedAddress(int module_internal_address)
      -> volatile uint32_t*;

 protected:
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> volatile uint32_t;
  AccelerationModule(MemoryManagerInterface* memory_manager,
                     int module_position)
      : memory_manager_(memory_manager),
        module_position_(module_position){};

 public:
  virtual ~AccelerationModule() = 0;
};
