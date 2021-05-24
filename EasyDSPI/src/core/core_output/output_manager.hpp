#pragma once

#include "output_manager_interface.hpp"

using easydspi::core_interfaces::OutputManagerInterface;

namespace easydspi::core::core_output {

class OutputManager : public OutputManagerInterface {
 public:
  ~OutputManager() override = default;

  std::vector<std::string> parse(
      std::vector<std::string> result_filenames) override;
};
}  // namespace easydspi::core::core_output