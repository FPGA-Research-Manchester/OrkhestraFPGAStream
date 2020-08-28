#pragma once
#include "query_acceleration_constants.hpp"
#include <vector>
struct DMACrossbarSetupData {
  DMACrossbarSetupData()
      : chunk_data(query_acceleration_constants::kDatapathWidth),
        position_data(query_acceleration_constants::kDatapathWidth) {}
  // TODO(Kaspar): Think about these names!
  std::vector<int> chunk_data;
  std::vector<int> position_data;
};