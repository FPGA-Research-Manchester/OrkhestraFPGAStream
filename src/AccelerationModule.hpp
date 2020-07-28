#pragma once
#include <cstdint>
class AccelerationModule {
 private:
  int* volatile const controlAXIbaseAddress_;
  const uint32_t modulePosition_;

  auto calculateMemoryMappedAddress(uint32_t module_internal_address)
      -> int* volatile;

 protected:
  void writeToModule(uint32_t module_internal_address, uint32_t write_data);
  auto readFromModule(uint32_t module_internal_address) -> uint32_t;
  AccelerationModule(int* volatile control_ax_ibase_address,
                     uint32_t module_position);

 public:
  virtual ~AccelerationModule() = 0;
};