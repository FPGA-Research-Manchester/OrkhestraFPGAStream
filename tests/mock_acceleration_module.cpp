#include "mock_acceleration_module.hpp"

#include <cstdint>

MockAccelerationModule::MockAccelerationModule(
    volatile int* ctrl_axi_base_address, int module_position)
    : AccelerationModule(ctrl_axi_base_address, module_position) {}
MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::WriteToModule(int module_internal_address,
                                           int write_data) {
  AccelerationModule::WriteToModule(module_internal_address, write_data);
}

auto MockAccelerationModule::ReadFromModule(int module_internal_address)
    ->  int {
  return AccelerationModule::ReadFromModule(module_internal_address);
}
