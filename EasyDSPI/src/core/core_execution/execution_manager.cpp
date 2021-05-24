#include "execution_manager.hpp"

using easydspi::core::core_execution::ExecutionManager;

std::vector<std::string> ExecutionManager::execute(
    std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
    execution_input) {
  return {execution_input.first->exportData(), execution_input.second.driver_library["1"]};
}