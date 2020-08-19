#include "acceleration_module.hpp"
#include <iostream>

//#include <unistd.h>

#define MODULE_ADDRESS_BITS 20

AccelerationModule::AccelerationModule(volatile int* control_axi_base_address,
                                       int module_position)
    : control_axi_base_address_{control_axi_base_address},
      module_position_{module_position} {}

auto AccelerationModule::CalculateMemoryMappedAddress(
    int module_internal_address) -> volatile int* {
  uintptr_t return_address =
      reinterpret_cast<uintptr_t>(control_axi_base_address_);
  return_address += module_position_ *
                    (1 << MODULE_ADDRESS_BITS);  // calculate the main address
                                                 // of the target module
  return_address += module_internal_address;
  return reinterpret_cast< int*>(return_address);
}

void AccelerationModule::WriteToModule(
    int module_internal_address,  // Internal address of the memory mapped
                                       // register of the module
    int write_data  // Data to be written to module's register
) {
  volatile int* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  std::cout<<"Address:"<<register_address<<" Data:"<<write_data<<std::endl;
  //usleep(1000);
  *register_address = write_data;
}

auto AccelerationModule::ReadFromModule(
    int module_internal_address  // Internal address of the memory mapped
                                      // register of the module
    ) -> volatile int {
  volatile int* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  return *register_address;
}

AccelerationModule::~AccelerationModule() = default;
