#pragma once
#include <cstdint>
class AccelerationModule {
 private:
  volatile uint32_t* const control_axi_base_address_;
  const int module_position_;

  auto CalculateMemoryMappedAddress(int module_internal_address)
      -> volatile uint32_t*;

 protected:
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> volatile uint32_t;
  AccelerationModule(volatile uint32_t* control_axi_base_address,
                     int module_position);

 public:
  virtual ~AccelerationModule() = 0;
};
