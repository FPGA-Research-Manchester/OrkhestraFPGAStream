#include "acceleration_module.hpp"
#include <iostream>

//#include <unistd.h>

#define MODULE_ADDRESS_BITS 20

AccelerationModule::AccelerationModule(unsigned int volatile control_axi_base_address,
                                       uint32_t module_position)
    : control_axi_base_address_{control_axi_base_address},
      module_position_{module_position} {}

auto AccelerationModule::CalculateMemoryMappedAddress(
    uint32_t module_internal_address) -> int* volatile {
  unsigned int volatile return_address = control_axi_base_address_;
  return_address += module_position_ *
                    (1 << MODULE_ADDRESS_BITS);  // calculate the main address
                                                 // of the target module
  return_address += module_internal_address;
  return reinterpret_cast<int*>(return_address);
}

void AccelerationModule::WriteToModule(
    uint32_t module_internal_address,  // Internal address of the memory mapped
                                       // register of the module
    uint32_t write_data  // Data to be written to module's register
) {
  int* volatile register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  std::cout<<"Address:"<<register_address<<" Data:"<<write_data<<std::endl;
  //usleep(1000);
  *register_address = write_data;
}

auto AccelerationModule::ReadFromModule(
    uint32_t module_internal_address  // Internal address of the memory mapped
                                      // register of the module
    ) -> uint32_t {
  volatile int* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  uint32_t read_data = *register_address;
  return read_data;
}

AccelerationModule::~AccelerationModule() = default;
