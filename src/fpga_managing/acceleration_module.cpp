#include "acceleration_module.hpp"

#include "fos/cynq.h"

AccelerationModule::AccelerationModule(StaticAccelInst* acceleration_instance,
                                       int module_position)
    : acceleration_instance_{acceleration_instance},
      module_position_{module_position} {}

auto AccelerationModule::CalculateMemoryMappedAddress(
    int module_internal_address) -> volatile uint32_t* {
  uintptr_t return_address =
      (1024 * 1024) * module_position_;  // calculate the main address
                                         // of the target module
  return_address += module_internal_address;
  return &(acceleration_instance_->prmanager->accelRegs[return_address / 4]);
}

void AccelerationModule::WriteToModule(
    int module_internal_address,  // Internal address of the memory mapped
                                  // register of the module
    uint32_t write_data           // Data to be written to module's register
) {
  volatile uint32_t* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  *register_address = write_data;
}

auto AccelerationModule::ReadFromModule(
    int module_internal_address  // Internal address of the memory mapped
                                 // register of the module
    ) -> volatile uint32_t {
  volatile uint32_t* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  return *register_address;
}

AccelerationModule::~AccelerationModule() = default;
