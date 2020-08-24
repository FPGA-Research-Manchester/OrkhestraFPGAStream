#include "acceleration_module.hpp"

#include <iostream>

//#include <unistd.h>

AccelerationModule::AccelerationModule(
    volatile uint32_t* control_axi_base_address, int module_position)
    : control_axi_base_address_{control_axi_base_address},
      module_position_{module_position} {}

auto AccelerationModule::CalculateMemoryMappedAddress(
    int module_internal_address) -> volatile uint32_t* {
  auto return_address = reinterpret_cast<uintptr_t>(control_axi_base_address_);
  return_address +=
      (1024 * 1024) * module_position_;  // calculate the main address
                                         // of the target module
  return_address += module_internal_address;
  return reinterpret_cast<volatile uint32_t*>(return_address);
}

void AccelerationModule::WriteToModule(
    int module_internal_address,  // Internal address of the memory mapped
                                  // register of the module
    uint32_t write_data           // Data to be written to module's register
) {
  volatile uint32_t* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  //std::cout << "Address:" << reinterpret_cast<uintptr_t>(register_address)
  //          << " Data:" << write_data << std::endl;
  // usleep(1000);
  //*register_address = write_data;
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
