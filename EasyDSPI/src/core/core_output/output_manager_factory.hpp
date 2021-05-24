#pragma once

#include <memory>
#include "output_manager_interface.hpp"

using easydspi::core_interfaces::OutputManagerInterface;

namespace easydspi::core::core_output {
class OutputManagerFactory {
 public:
  static std::unique_ptr<OutputManagerInterface> getManager();
};
}  // namespace easydspi::core::core_execution