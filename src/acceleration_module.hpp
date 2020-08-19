#pragma once
#include <cstdint>
class AccelerationModule {
 private:
  volatile int* const control_axi_base_address_;
  const int module_position_;

  auto CalculateMemoryMappedAddress(int module_internal_address)
      -> volatile int*;

 protected:
  void WriteToModule(int module_internal_address, int write_data);
  auto ReadFromModule(int module_internal_address) -> volatile int;
  AccelerationModule(volatile int* control_axi_base_address,
                     int module_position);

 public:
  virtual ~AccelerationModule() = 0;
};
