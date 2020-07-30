#pragma once
#include <vector>

class Setup {
 public:
  static void SetupQueryAcceleration(int* volatile& memory_pointer,
                                     std::vector<int>& db_data, int record_size,
                                     int record_count);
};
