#pragma once
#include <cstdint>

class Setup {
 public:
  static void SetupQueryAcceleration(
      volatile uint32_t* configuration_memory_address,
      volatile uint32_t* input_memory_address,
      volatile uint32_t* output_memory_address, int record_size,
      int record_count);
};
