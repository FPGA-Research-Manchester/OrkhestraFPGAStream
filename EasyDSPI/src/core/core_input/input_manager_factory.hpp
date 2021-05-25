#pragma once

#include <memory>

#include "input_manager_interface.hpp"
#include "input_manager_config_enums.hpp"

using easydspi::core_interfaces::InputManagerInterface;

namespace easydspi::core::core_input {
class InputManagerFactory {
 public:
  static std::unique_ptr<InputManagerInterface> getManager(
      InputManagerValidationEnum manager_type);
};
}  // namespace easydspi::core::core_input