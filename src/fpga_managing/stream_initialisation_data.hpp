#pragma once
#include <memory>
#include <string>

#include "memory_block_interface.hpp"

namespace dbmstodspi::fpga_managing {

/**
 * @brief Struct to hold data from which the configuration data can be
 * calculated before FPGA configuration
 */
struct StreamInitialisationData {
  /// ID
  int stream_id;
  /// Data CSV file location.
  std::string stream_data_file_name;
  /// Memory mapped DDR location where the data will be written to.
  std::unique_ptr<MemoryBlockInterface> memory_block;
};

}  // namespace dbmstodspi::fpga_managing