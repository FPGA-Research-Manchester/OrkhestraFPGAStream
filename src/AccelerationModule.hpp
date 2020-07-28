#pragma once
#include <cstdint>
class AccelerationModule {
 private:
  int* volatile const controlAXIbaseAddress_;
  const uint32_t modulePosition_;

  auto calculateMemoryMappedAddress(uint32_t moduleInternalAddress)
      -> int* volatile;

 protected:
  void writeToModule(uint32_t moduleInternalAddress, uint32_t writeData);
  auto readFromModule(uint32_t moduleInternalAddress) -> uint32_t;
  AccelerationModule(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);

 public:
  virtual ~AccelerationModule() = 0;
};