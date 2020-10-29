#pragma once
#include <cstdint>

#include "acceleration_module.hpp"
#include "memory_manager.hpp"

class MockAccelerationModule : public AccelerationModule {
 public:
  MockAccelerationModule(MemoryManager* memory_manager, int module_position);
  ~MockAccelerationModule() override;
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> uint32_t;
};