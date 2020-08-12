#pragma once
#include <vector>

class Setup {
 public:
  static void SetupQueryAcceleration(unsigned int volatile memory_pointer,

                                     std::vector<int>& db_data,
                                     int* volatile output_memory_address,
                                     int record_size, int record_count);
};
