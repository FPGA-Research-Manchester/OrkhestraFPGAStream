#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "execution_plan_graph_interface.hpp"

namespace easydspi::core_interfaces {
class ExecutionManagerInterface {
 public:
  virtual ~ExecutionManagerInterface() = default;
  virtual std::vector<std::string> execute(
      std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
          execution_input) = 0;
};
}  // namespace easydspi::core_interfaces