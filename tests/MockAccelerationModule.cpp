#include "MockAccelerationModule.hpp"

#include <cstdint>

MockAccelerationModule::MockAccelerationModule(
    int* volatile ctrl_ax_ibase_address, uint32_t module_position)
    : AccelerationModule(ctrl_ax_ibase_address, module_position) {}
MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::writeToModule(uint32_t module_internal_address,
                                           uint32_t write_data) {
  AccelerationModule::WriteToModule(module_internal_address, write_data);
}

auto MockAccelerationModule::readFromModule(uint32_t module_internal_address)
    -> uint32_t {
  return AccelerationModule::ReadFromModule(module_internal_address);
}
