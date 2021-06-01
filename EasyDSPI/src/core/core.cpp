#include "core.hpp"

#include <memory>

#include "core_execution/execution_manager_factory.hpp"
#include "core_input/input_manager_factory.hpp"
#include "input_manager_config_enums.hpp"
#include "input_manager_interface.hpp"

using easydspi::core::Core;
using easydspi::core::core_execution::ExecutionManagerFactory;
using easydspi::core::core_input::InputManagerFactory;
using easydspi::core::core_input::InputManagerValidationEnum;

using easydspi::core_interfaces::InputManagerInterface;

std::vector<std::string> Core::run(std::string input_filename,
                                   std::string config_filename) {
  // Could be done as a one liner but additional future logic is expected here
  // to be passed to the factories.
  auto input =
      InputManagerFactory::getManager(InputManagerValidationEnum::kNoValidation)
          ->parse(input_filename, config_filename);
  return ExecutionManagerFactory::getManager()->execute(std::move(input));
}