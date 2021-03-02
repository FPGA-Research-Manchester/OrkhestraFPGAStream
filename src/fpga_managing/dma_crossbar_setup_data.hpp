#pragma once
#include "query_acceleration_constants.hpp"
#include <vector>
struct DMACrossbarSetupData {
  DMACrossbarSetupData()
      : chunk_selection(query_acceleration_constants::kDatapathWidth,
                        (query_acceleration_constants::kDatapathLength - 1)),
        position_selection(query_acceleration_constants::kDatapathWidth, 0) {}
  std::vector<int> chunk_selection;
  std::vector<int> position_selection;
};