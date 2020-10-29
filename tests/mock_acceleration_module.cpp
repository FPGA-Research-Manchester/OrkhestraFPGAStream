#include "mock_acceleration_module.hpp"

#include <cstdint>

MockAccelerationModule::MockAccelerationModule(MemoryManager* memory_manager,
                                               int module_position)
    : AccelerationModule(memory_manager, module_position) {}
MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::WriteToModule(int module_internal_address,
                                           uint32_t write_data) {
  AccelerationModule::WriteToModule(module_internal_address, write_data);
}

auto MockAccelerationModule::ReadFromModule(int module_internal_address)
    -> uint32_t {
  return AccelerationModule::ReadFromModule(module_internal_address);
}
