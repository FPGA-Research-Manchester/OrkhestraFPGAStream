#include "AccelerationModule.hpp"

#define MODULE_ADDRESS_BITS 20

AccelerationModule::AccelerationModule(int* volatile controlAXIbaseAddress,
                                       uint32_t modulePosition)
    : controlAXIbaseAddress_{controlAXIbaseAddress},
      modulePosition_{modulePosition} {}

auto AccelerationModule::calculateMemoryMappedAddress(
    uint32_t moduleInternalAddress) -> int* volatile {
  int* volatile return_address = controlAXIbaseAddress_;
  return_address += modulePosition_ *
                    (1 << MODULE_ADDRESS_BITS);  // calculate the main address
                                                 // of the target module
  return_address += moduleInternalAddress;
  return return_address;
}

void AccelerationModule::writeToModule(
    uint32_t moduleInternalAddress,  // Internal address of the memory mapped
                                     // register of the module
    uint32_t writeData               // Data to be written to module's register
) {
  int* volatile register_address =
      calculateMemoryMappedAddress(moduleInternalAddress);
  *register_address = writeData;
}

auto AccelerationModule::readFromModule(
    uint32_t moduleInternalAddress  // Internal address of the memory mapped
                                    // register of the module
    ) -> uint32_t {
  volatile int* register_address =
      calculateMemoryMappedAddress(moduleInternalAddress);
  uint32_t read_data = *register_address;
  return read_data;
}

AccelerationModule::~AccelerationModule() = default;
