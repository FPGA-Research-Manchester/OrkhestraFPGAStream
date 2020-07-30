#pragma once
#include <vector>
struct DMACrossbarSetupData {
  DMACrossbarSetupData() : chunk_data(16), position_data(16) {}
  // TODO(Kaspar): Think about these names!
  std::vector<int> chunk_data;
  std::vector<int> position_data;
};