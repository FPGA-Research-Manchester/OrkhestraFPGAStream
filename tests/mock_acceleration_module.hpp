#pragma once
#include <cstdint>

#include "acceleration_module.hpp"

class MockAccelerationModule : public AccelerationModule {
 public:
  MockAccelerationModule(volatile int* ctrl_axi_base_address,
                         int module_position);
  ~MockAccelerationModule() override;
  void WriteToModule(int module_internal_address, int write_data);
  auto ReadFromModule(int module_internal_address) ->  int;
};