#pragma once
#include <cstdint>

#include "acceleration_module.hpp"

class MockAccelerationModule : public AccelerationModule {
 public:
  MockAccelerationModule(int* volatile ctrl_axi_base_address,
                         uint32_t module_position);
  ~MockAccelerationModule() override;
  void writeToModule(uint32_t module_internal_address, uint32_t write_data);
  auto readFromModule(uint32_t module_internal_address) -> uint32_t;
};