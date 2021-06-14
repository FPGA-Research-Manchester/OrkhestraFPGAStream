#include "acceleration_module.hpp"

#include <iostream>

#include "logger.hpp"

using namespace dbmstodspi::fpga_managing::modules;

auto AccelerationModule::CalculateMemoryMappedAddress(
    int module_internal_address) -> volatile uint32_t* {
  uintptr_t address_offset =
      (1024 * 1024) * module_position_;  // calculate the main address
                                         // of the target module
  address_offset += module_internal_address;
  return memory_manager_->GetVirtualRegisterAddress(address_offset);
}

void AccelerationModule::WriteToModule(
    int module_internal_address,  // Internal address of the memory mapped
                                  // register of the module
    uint32_t write_data           // Data to be written to module's register
) {
  volatile uint32_t* register_address =
      CalculateMemoryMappedAddress(module_internal_address);
  auto log_level = dbmstodspi::logger::LogLevel::kTrace;
  if (dbmstodspi::logger::ShouldLog(log_level)) {
    std::stringstream ss;
    ss << std::hex << "Module: " << module_position_
       << " Address: " << module_internal_address << " Data: " << write_data
       << std::endl;
    dbmstodspi::logger::Log(log_level, ss.str());
  }
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
