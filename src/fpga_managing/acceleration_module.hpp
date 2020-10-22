#pragma once
#include <cstdint>

#include "cynq.h"

class AccelerationModule {
 private:
  StaticAccelInst* acceleration_instance_;
  const int module_position_;

  auto CalculateMemoryMappedAddress(int module_internal_address)
      -> volatile uint32_t*;

 protected:
  void WriteToModule(int module_internal_address, uint32_t write_data);
  auto ReadFromModule(int module_internal_address) -> volatile uint32_t;
  AccelerationModule(StaticAccelInst* acceleration_instance,
                     int module_position);

 public:
  virtual ~AccelerationModule() = 0;
};
