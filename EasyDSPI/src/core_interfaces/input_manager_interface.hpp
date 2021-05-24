#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "execution_plan_graph_interface.hpp"

namespace easydspi::core_interfaces {
class InputManagerInterface {
 public:
  virtual ~InputManagerInterface() = default;
  virtual std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config> parse(
      std::string input_filename, std::string config_filename) = 0;
};
}  // namespace easydspi::core::core_interfaces