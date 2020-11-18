#pragma once
#include <cstdint>

#include "acceleration_module.hpp"
#include "memory_manager_interface.hpp"

class MockAccelerationModule : public AccelerationModule {
 public:
  MockAccelerationModule(MemoryManagerInterface* memory_manager,
                         int module_position)
      : AccelerationModule(memory_manager, module_position) {}
  ~MockAccelerationModule() override;
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> uint32_t;
};