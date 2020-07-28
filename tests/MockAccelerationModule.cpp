#include "MockAccelerationModule.hpp"

#include <cstdint>

MockAccelerationModule::MockAccelerationModule(int* volatile ctrlAXIbaseAddress,
                                               uint32_t modulePosition)
    : AccelerationModule(ctrlAXIbaseAddress, modulePosition) {}
MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::writeToModule(uint32_t moduleInternalAddress,
                                           uint32_t writeData) {
  AccelerationModule::writeToModule(moduleInternalAddress, writeData);
}

auto MockAccelerationModule::readFromModule(uint32_t moduleInternalAddress)
    -> uint32_t {
  return AccelerationModule::readFromModule(moduleInternalAddress);
}
