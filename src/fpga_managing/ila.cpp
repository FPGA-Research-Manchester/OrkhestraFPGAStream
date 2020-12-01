#include "ila.hpp"

void ILA::startILAs() { 
	ILA::WriteToModule(0x10000000, 1); 
	ILA::WriteToModule(0x10000004, 3);
    ILA::WriteToModule(0x12000000, 1);
    ILA::WriteToModule(0x12000004, 3);
}

void ILA::startAxiILA() { ILA::WriteToModule(0x14000000, 1); }

auto ILA::getValues(int clock_cycle, int location, ILADataTypes data_type)
    -> uint32_t {
  return ILA::ReadFromModule(
      ILA::calcAddress(clock_cycle, location, static_cast<int>(data_type)));
}

auto ILA::calcAddress(int clock, int location, int offset) -> int {
  if (location == 0) {
    return (0x10000000 + (clock << 11) + (offset << 2));
  }
  if (location == 1) {
    return (0x12000000 + (clock << 11) + (offset << 2));
  }
  if (location == 2) {
    return (0x14000000 + (clock << 11) + (offset << 2));
  }
}

void ILA::WriteToModule(int module_internal_address, uint32_t write_data) {
  volatile uint32_t* register_address =
      memory_manager_->GetVirtualRegisterAddress(module_internal_address);
  *register_address = write_data;
}

auto ILA::ReadFromModule(int module_internal_address) -> uint32_t {
  volatile uint32_t* register_address =
      memory_manager_->GetVirtualRegisterAddress(module_internal_address);
  return *register_address;
}