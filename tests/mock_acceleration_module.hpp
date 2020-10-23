#pragma once
#include <cstdint>
#include "cynq.h"

#include "acceleration_module.hpp"

class MockAccelerationModule : public AccelerationModule {
 public:
  MockAccelerationModule(StaticAccelInst* acceleration_instance,
                         int module_position);
  ~MockAccelerationModule() override;
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> uint32_t;
};