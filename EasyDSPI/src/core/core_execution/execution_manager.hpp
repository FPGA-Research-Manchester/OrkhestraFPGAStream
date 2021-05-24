#pragma once

#include "execution_manager_interface.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionManagerInterface;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::core::core_execution {

class ExecutionManager : public ExecutionManagerInterface {
 public:
  ~ExecutionManager() override = default;

  std::vector<std::string> execute(
      std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
          execution_input) override;
};
}  // namespace easydspi::core::core_execution