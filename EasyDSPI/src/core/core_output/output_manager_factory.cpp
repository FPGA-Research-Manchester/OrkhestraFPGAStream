#include "output_manager_factory.hpp"

#include "output_manager.hpp"

using easydspi::core::core_output::OutputManager;
using easydspi::core::core_output::OutputManagerFactory;
using easydspi::core_interfaces::OutputManagerInterface;

std::unique_ptr<OutputManagerInterface> OutputManagerFactory::getManager() {
  return std::make_unique<OutputManager>();
}