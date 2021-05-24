#pragma once

#include <memory>
#include "execution_manager_interface.hpp"

using easydspi::core_interfaces::ExecutionManagerInterface;

namespace easydspi::core::core_execution {
class ExecutionManagerFactory {
 public:
  static std::unique_ptr<ExecutionManagerInterface> getManager();
};
}  // namespace easydspi::core::core_execution