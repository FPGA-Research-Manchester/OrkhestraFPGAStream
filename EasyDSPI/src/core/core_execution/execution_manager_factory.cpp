#include "execution_manager_factory.hpp"

#include "execution_manager.hpp"

using easydspi::core::core_execution::ExecutionManager;
using easydspi::core::core_execution::ExecutionManagerFactory;
using easydspi::core_interfaces::ExecutionManagerInterface;

std::unique_ptr<ExecutionManagerInterface>
ExecutionManagerFactory::getManager() {
  return std::make_unique<ExecutionManager>();
}