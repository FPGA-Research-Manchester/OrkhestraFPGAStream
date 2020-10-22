#include "mock_acceleration_module.hpp"

#include <cstdint>

MockAccelerationModule::MockAccelerationModule(
    StaticAccelInst* acceleration_instance, int module_position)
    : AccelerationModule(acceleration_instance, module_position) {}
MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::WriteToModule(int module_internal_address,
                                           uint32_t write_data) {
  AccelerationModule::WriteToModule(module_internal_address, write_data);
}

auto MockAccelerationModule::ReadFromModule(int module_internal_address)
    -> uint32_t {
  return AccelerationModule::ReadFromModule(module_internal_address);
}
