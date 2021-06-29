#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "table_data.hpp"

using dbmstodspi::data_managing::table_data::ColumnDataType;

/// Can be change to have a dictionary of config file names to make it more
/// generic
namespace dbmstodspi::input_managing {
struct Config {
  // Constructed run time for now. So doesn't even really need to be here.
  // std::map<std::string, std::vector<std::vector<int>>> module_library;
  std::map<fpga_managing::operation_types::QueryOperationType,
           std::vector<std::vector<int>>>
      module_library;

  // std::map<std::vector<std::pair<std::string, std::vector<int>>>,
  // std::string>
  std::map<std::vector<fpga_managing::operation_types::QueryOperation>,
           std::string>
      accelerator_library;

  std::map<std::string, int> required_memory_space;
  std::map<ColumnDataType, double> data_sizes;
  // std::map<std::string, std::string> driver_library;
  char separator;
};
}  // namespace dbmstodspi::input_managing