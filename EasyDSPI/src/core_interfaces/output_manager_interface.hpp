#pragma once

#include <string>
#include <vector>

// Class which really isn't needed. Just to demonstrate the an output processing step is easy to add!
namespace easydspi::core_interfaces {
class OutputManagerInterface {
 public:
  virtual ~OutputManagerInterface() = default;
  virtual std::vector<std::string> parse(std::vector<std::string> result_filenames) = 0;
};
}  // namespace easydspi::core::core_interfaces
