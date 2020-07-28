#include "AccelerationModule.hpp"

#define MODULE_ADDRESS_BITS 20

AccelerationModule::AccelerationModule(int* volatile controlAXIbaseAddress,
                                       uint32_t modulePosition)
    : controlAXIbaseAddress_{controlAXIbaseAddress},
      modulePosition_{modulePosition} {}

auto AccelerationModule::calculateMemoryMappedAddress(
    uint32_t moduleInternalAddress) -> int* volatile {
  int* volatile returnAddress = controlAXIbaseAddress_;
  returnAddress += modulePosition_ *
                   (1 << MODULE_ADDRESS_BITS);  // calculate the main address of
                                                // the target module
  returnAddress += moduleInternalAddress;
  return returnAddress;
}

void AccelerationModule::writeToModule(
    uint32_t moduleInternalAddress,  // Internal address of the memory mapped
                                     // register of the module
    uint32_t writeData               // Data to be written to module's register
) {
  int* volatile registerAddress =
      calculateMemoryMappedAddress(moduleInternalAddress);
  *registerAddress = writeData;
}

auto AccelerationModule::readFromModule(
    uint32_t moduleInternalAddress  // Internal address of the memory mapped
                                    // register of the module
    ) -> uint32_t {
  volatile int* registerAddress =
      calculateMemoryMappedAddress(moduleInternalAddress);
  uint32_t readData = *registerAddress;
  return readData;
}

AccelerationModule::~AccelerationModule() = default;
