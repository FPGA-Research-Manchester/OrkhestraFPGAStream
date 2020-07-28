#pragma once
#include <vector>

class Setup {
 public:
  static void SetupQueryAcceleration(int* volatile& memoryPointer,
                                     std::vector<int>& dbData, int recordSize,
                                     int recordCount);
};
