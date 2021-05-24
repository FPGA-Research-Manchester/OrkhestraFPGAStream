#include "input_manager_factory.hpp"

#include "graph_creator.hpp"
#include "input_manager.hpp"

using easydspi::core::core_input::InputManager;
using easydspi::core::core_input::InputManagerFactory;
using easydspi::core_interfaces::InputManagerInterface;
using easydspi::dbmstodspi::GraphCreator;

std::unique_ptr<InputManagerInterface> InputManagerFactory::getManager() {
  return std::make_unique<InputManager>(GraphCreator());
}