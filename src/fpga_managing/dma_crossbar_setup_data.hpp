#pragma once
#include <vector>

#include "query_acceleration_constants.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Struct to hold crossbar configuration data vectors.
 */
struct DMACrossbarSetupData {
  /**
   * @brief Constructor to initialise default configuration data.
   */
  DMACrossbarSetupData()
      : chunk_selection(query_acceleration_constants::kDatapathWidth,
                        (query_acceleration_constants::kDatapathLength - 1)),
        position_selection(query_acceleration_constants::kDatapathWidth, 0) {}
  /// Data which helps choose the chunk position.
  std::vector<int> chunk_selection;
  /// Data which helps choose the position within a chunk.
  std::vector<int> position_selection;
};

}  // namespace fpga_managing
}  // namespace dbmstodspi