#include "mock_acceleration_module.hpp"

#include <cstdint>

MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::WriteToModule(int module_internal_address,
                                           uint32_t write_data) {
  dbmstodspi::fpga_managing::AccelerationModule::WriteToModule(
      module_internal_address, write_data);
}

auto MockAccelerationModule::ReadFromModule(int module_internal_address)
    -> uint32_t {
  return dbmstodspi::fpga_managing::AccelerationModule::ReadFromModule(
      module_internal_address);
}
