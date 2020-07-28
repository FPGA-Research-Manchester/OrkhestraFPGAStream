#pragma once
#include <vector>
struct DMACrossbarSetupData {
  DMACrossbarSetupData() : chunkData(16), positionData(16) {}
  // TODO: Think about these names!
  std::vector<int> chunkData;
  std::vector<int> positionData;
};