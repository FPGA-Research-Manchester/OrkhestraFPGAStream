#pragma once

#include <string>
#include <map>
#include <vector>
#include <utility>

/// Can be change to have a dictionary of config file names to make it more generic

namespace easydspi::core_interfaces {
struct Config {
  // Constructed run time for now. So doesn't even really need to be here.
  std::map<std::string, std::vector<std::vector<int>>> module_library;

  std::map<std::vector<std::pair<std::string, std::vector<int>>>, std::string>
      accelerator_library;

  std::map<std::string, int> required_memory_space;
  // Maybe int?
  std::map<std::string, double> data_sizes;
  std::map<std::string, std::string> driver_library;
};
}  // namespace easydspi::core::core_interfaces