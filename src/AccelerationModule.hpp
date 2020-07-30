#pragma once
#include <cstdint>
class AccelerationModule {
 private:
  int* volatile const control_axi_base_address_;
  const uint32_t module_position_;

  auto CalculateMemoryMappedAddress(uint32_t module_internal_address)
      -> int* volatile;

 protected:
  void WriteToModule(uint32_t module_internal_address, uint32_t write_data);
  auto ReadFromModule(uint32_t module_internal_address) -> uint32_t;
  AccelerationModule(int* volatile control_ax_ibase_address,
                     uint32_t module_position);

 public:
  virtual ~AccelerationModule() = 0;
};