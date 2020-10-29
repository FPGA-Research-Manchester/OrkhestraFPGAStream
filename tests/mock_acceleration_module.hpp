#pragma once
#include <cstdint>
#include "memory_manager.hpp"

#include "acceleration_module.hpp"

class MockAccelerationModule : public AccelerationModule {
 public:
  MockAccelerationModule(MemoryManager* memory_manager,
                         int module_position);
  ~MockAccelerationModule() override;
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> uint32_t;
};