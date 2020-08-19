#pragma once
#include <vector>

class Setup {
 public:
  static void SetupQueryAcceleration(volatile int* memory_pointer,
                                     std::vector<int>& db_data,
                                     volatile int* output_memory_address,
                                     int record_size, int record_count);
};
